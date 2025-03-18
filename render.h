#pragma once

#include "constants.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "draw.h"

#include <fstream>

struct RayTraceResult {
    color pixel_color;
    double depth;

    RayTraceResult(const color& c, double d) : pixel_color(c), depth(d) {}
    RayTraceResult() : pixel_color(0,0,0), depth(infinity) {}
};

class camera {
    public:
        // public camera parameters
        double aspect_ratio;
        int image_width;
        int samples_per_pixel = 1;
        int max_depth = 2;
        color background = color(0,0,0);
        double fps = 0;     // render speed in fps
        int view_mode = 0;
        
        double vfov = 20;                   // vertical field of view
        point3 lookfrom = point3(1,1,3);    // point camera is looking from
        point3 lookat = point3(0,0,-1);     // point camera is looking at
        vec3 vup = vec3(0,1,0);             // camera-relative up direction
        // viewport dimensions
        double viewport_width;
        double viewport_height;
        
        double defocus_angle = 0;
        double focus_dist = 1;
        
        camera(): aspect_ratio(1.0), image_width(100) {
            initialize();
        }
        
        camera(double aspect_ratio, int image_width): aspect_ratio(aspect_ratio), image_width(image_width) {
            initialize();
        }
        
        int get_image_height() const {
            return image_height;
        }
        
        void render(const hittable& world, std::vector<u_int32_t>& buffer) {
            initialize();
            
            // create buffer for depth map
            std::vector<double> depth_buffer(image_width * image_height, infinity);
            
            // record start time
            auto start = std::chrono::high_resolution_clock::now();
            
            // render and write to buffer
            for (int j = 0; j < image_height; j++) {
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0,0,0);
                    double min_depth = infinity;
                    
                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j);
                        RayTraceResult result = ray_color(r, max_depth, world);
                        pixel_color += result.pixel_color;
                        min_depth = std::min(min_depth, result.depth);
                    }
                    
                    write_color(buffer, i, j, image_width, pixel_samples_scale * pixel_color);
                    depth_buffer[j * image_width + i] = min_depth;
                }
            }
            
            std::vector<aabb> bboxes;
            world.gather_internal_nodes(bboxes);
            
            for (const auto& box: bboxes) {
                draw_box_edges(box, buffer, depth_buffer);
            }
            
            // record end time
            auto end = std::chrono::high_resolution_clock::now();
            // calculate elapsed time
            std::chrono::duration<double> elapsed = end - start;
            double elapsed_seconds = elapsed.count();
            // calculate in fps
            fps = 1.0 / elapsed_seconds;
        }
        
        ray get_ray(int i, int j) const {
            // construct a camera ray originating from the origin and directed at a randomely
            // sampled point around the pixel at location (i, j)
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc
            + ((i + offset.x()) * pixel_delta_u)
            + ((j + offset.y()) * pixel_delta_v);
            
            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;
            
            return ray(ray_origin, ray_direction);
            
        }
        
        // Wrapper that selects between debug and normal views.
        RayTraceResult ray_color(const ray& r, int depth, const hittable& world) const {
            return (view_mode == 0) ? ray_color_rgb(r, depth, world) 
            : (view_mode == 1) ? ray_color_debug(r, depth, world) 
            : ray_color_bvh(r, depth, world);
            
        }
        
        void project_to_viewport(const point3& p, point3& screen_coords) {
            // vector pointing from camera origin to point p
            vec3 OP = p - lookfrom;
            // project OP onto viewport basis vectors u, v and w
            auto viewport_x = dot(OP, u);
            auto viewport_y = dot(OP, v);
            auto viewport_z = dot(OP, -w);

            // // Perspective projection
            // if (viewport_z <= 0) {
            //     // Point is behind camera
            // screen_coords = vec3(-1, -1, infinity);
            // return;
            // }
            
            // map to normalized device coordinates
            auto scale_factor = 1 / viewport_z;
            double ndc_x = viewport_x * scale_factor / (viewport_width / 2); 
            double ndc_y = viewport_y * scale_factor / (viewport_height / 2);
            
            // Convert to screen coordinates
            int screen_x = (ndc_x + 1) * 0.5 * image_width;
            int screen_y = (1 - ndc_y) * 0.5 * image_height;
            
            screen_coords = vec3(screen_x, screen_y, viewport_z);
            
        }

        void draw_box_edges(
            const aabb& box, 
            std::vector<uint32_t>& color_buffer,
            std::vector<double>& depth_buffer) {
                
            // The 8 corners of the box
            point3 corners[8] = {
                point3(box.x.min, box.y.min, box.z.max),
                point3(box.x.max, box.y.min, box.z.max),
                point3(box.x.min, box.y.min, box.z.min),
                point3(box.x.max, box.y.min, box.z.min),
                point3(box.x.min, box.y.max, box.z.max),
                point3(box.x.max, box.y.max, box.z.max),
                point3(box.x.min, box.y.max, box.z.min),
                point3(box.x.max, box.y.max, box.z.min)
            };
            
            // The 12 edges defined by pairs of corner indices
            int edges[12][2] = {
                {0, 1}, {0, 2}, {1, 3}, {2, 3}, // Bottom face
                {4, 5}, {4, 6}, {5, 7}, {6, 7}, // Top face
                {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Connecting edges
            };
        
            point3 screen_corners[8];
            for (int i = 0; i < 8; ++i) {
                project_to_viewport(corners[i], screen_corners[i]);
            }

            // draw edges
            for (int i = 0; i < 12; ++i) {
                point3& p0 = screen_corners[edges[i][0]];
                point3& p1 = screen_corners[edges[i][1]];

                // Skip lines if either point is behind the camera
                if (p0.z() <= 0 || p1.z() <= 0) {
                    continue;
                }

                bresenham(p0, p1, color_buffer, depth_buffer, image_width, image_height);
            }
        }
        
        vec3 sample_square() const {
            // returns a vector in the [-.5,-.5] - [+.5,+.5] unit square
            return vec3(random_double() - 0.5, random_double() - 0.5, 0);
        }   
        
        point3 defocus_disk_sample() const {
            // returns a random point in the camera defocus disk
            auto p = random_in_unit_disk();
            return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }
        
        private:
        int image_height;           // rendered image height
        double pixel_samples_scale; // scaling factor for rgb values
        point3 center;              // camera center
        point3 pixel00_loc;         // location of pixel (0, 0) 
        vec3 u, v, w;               // camera frame basis vectors
        vec3 pixel_delta_u;         // pixel spacing in horizontal
        vec3 pixel_delta_v;         // pixel spacing in vertical direction
        vec3 defocus_disk_u;        // defocus disk horizontal radius
        vec3 defocus_disk_v;        // defocus deisk vertical radius
        
        void initialize() {
            image_height = int(image_width / aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height;
            
            // calculate scaling factor for rgb values
            pixel_samples_scale = (view_mode == 1) ? 1.0 / (samples_per_pixel * max_depth)
                                                   : 1.0 / samples_per_pixel;

            center = lookfrom;

            // viewport parameters
            auto theta = degrees_to_radians(vfov);
            viewport_height = 2 * std::tan(theta/2) * focus_dist;
            viewport_width = viewport_height * (double(image_width)/image_height);
            
            // calculate u, v, w basis vectors
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);

            // calculate vectros across horizontal and down vertical viewport edges
            auto viewport_u = viewport_width * u;       // across horizontal viewport edge
            auto viewport_v = viewport_height * -v;     // down view vertical edge
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;
            auto viewport_upper_left = center 
                                        - (focus_dist * w)
                                        - viewport_u / 2
                                        - viewport_v / 2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

            // Calculate the camera defocus disk basis vectors.
            auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
            defocus_disk_u = u * defocus_radius;
            defocus_disk_v = v * defocus_radius;
        }

        RayTraceResult ray_color_rgb(const ray& r, int depth, const hittable& world) const {
            color accumulated_color(0, 0, 0);
            color current_attenuation(1, 1, 1);
            ray current_ray = r;
            double min_depth = infinity;
        
            // Iterate up to the maximum bounce depth.
            for (int i = 0; i < depth; i++) {
                hit_record rec;
                // If no hit, add background color modulated by the current attenuation.
                if (!world.hit(current_ray, interval(0.001, infinity), rec)) {
                    accumulated_color += current_attenuation * background;
                    break;
                }
        
                // Add emission from the hit point.
                accumulated_color += current_attenuation * rec.mat->emitted(rec.u, rec.v, rec.p);
        
                ray scattered;
                color scatter_attenuation;
                // If the material does not scatter, we end the loop.
                if (!rec.mat->scatter(current_ray, rec, scatter_attenuation, scattered)) {
                    break;
                }
        
                // Update the attenuation and continue with the scattered ray.
                current_attenuation *= scatter_attenuation;
                current_ray = scattered;
            }
            return RayTraceResult(accumulated_color, min_depth);
        }
    
        // Debug view: returns a grayscale color representing the number of hits.
        RayTraceResult ray_color_debug(const ray& r, int depth, const hittable& world) const {
            int hit_test = 0;
            ray current_ray = r;
            double min_depth = infinity;

            for (int i = 0; i < depth; ++i) {
                hit_record rec;
                ++hit_test;
                if (world.hit(current_ray, interval(0.001, infinity), rec)) {
                    ray scattered;
                    color attenuation;
                    if (rec.mat->scatter(current_ray, rec, attenuation, scattered)) {
                        current_ray = scattered;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
            double ratio = static_cast<double>(hit_test);
            return RayTraceResult(color(ratio, ratio, ratio), min_depth);
        }
        
        RayTraceResult ray_color_bvh(const ray& r, int depth, const hittable& world) const {
            // to do:
            // if hit scatter object -> return color depending on angle relative to normal
            // if hit emissive object -> return color emissive object
            ray current_ray = r;
            double min_depth = infinity;
            hit_record rec;

            if (!world.hit(r, interval(0.001, infinity), rec))
                return RayTraceResult();
            // get intersection point and calculate OP. then dot with -w and compare against depth map
            min_depth = rec.t;

            double near_plane = 0.1;
            double far_plane =  20.0;
            double normalized_depth = (min_depth - near_plane) / (far_plane - near_plane);
            
            normalized_depth = interval(0.0, 1.0).clamp(normalized_depth);
            color depth_color = color(normalized_depth, normalized_depth, normalized_depth);

            return RayTraceResult(depth_color, min_depth);
        }
};

void write_to_ppm(int image_width, int image_height, const std::vector<uint32_t>& buffer, const std::string& filename) {
    std::ofstream out(filename);
    out << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
            uint32_t pixel = buffer[j * image_width + i];
            
            // Extract from ABGR
            int b = (pixel >> 16) & 0xFF;
            int g = (pixel >> 8)  & 0xFF;
            int r =  pixel        & 0xFF;
            
            // Write as RGB (PPM format)
            out << r << ' ' << g << ' ' << b << '\n';
        }
    }
    out.close();
}
