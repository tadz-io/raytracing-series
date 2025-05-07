#pragma once

#include "constants.h"
#include "hittable.h"
#include "hittable_list.h"
#include "pdf.h"
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
        
        void render(const hittable& world, const hittable& lights, std::vector<u_int32_t>& buffer) {
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
                    
                    for (int s_j = 0; s_j < sqrt_spp; s_j++) {
                        for (int s_i = 0; s_i < sqrt_spp; s_i++){
                            ray r = get_ray(i, j, s_i, s_j);
                            RayTraceResult result = ray_color(r, max_depth, world, lights);
                            pixel_color += result.pixel_color;
                            min_depth = std::min(min_depth, result.depth);
                        }
                    }
                    
                    write_color(buffer, i, j, image_width, pixel_samples_scale * pixel_color);
                    depth_buffer[j * image_width + i] = min_depth;
                }
            }
            
            // show bvh edges in bvh mode
            if (view_mode == 2) {
                std::vector<aabb> bboxes;
                world.gather_internal_nodes(bboxes);
                
                for (const auto& box: bboxes) {
                    draw_box_edges(box, buffer, depth_buffer);
                }
            }

            // record end time
            auto end = std::chrono::high_resolution_clock::now();
            // calculate elapsed time
            std::chrono::duration<double> elapsed = end - start;
            double elapsed_seconds = elapsed.count();
            // calculate in fps
            fps = 1.0 / elapsed_seconds;
        }
        
        ray get_ray(int i, int j, int s_i, int s_j) const {
            // construct a camera ray originating from the origin and directed at a randomely
            // sampled point around the pixel at location (i, j)
            auto offset = sample_square_stratified(s_i, s_j);
            auto pixel_sample = pixel00_loc
            + ((i + offset.x()) * pixel_delta_u)
            + ((j + offset.y()) * pixel_delta_v);
            
            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;
            
            return ray(ray_origin, ray_direction);
            
        }

        void initialize() {
            image_height = int(image_width / aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height;
            
            sqrt_spp = int(std::sqrt(samples_per_pixel));
            recip_sqrt_spp = 1.0 / sqrt_spp;
            // calculate scaling factor for rgb values
            pixel_samples_scale = (view_mode == 1) ? 1.0 / (sqrt_spp * sqrt_spp * max_depth)
                                                   : 1.0 / (sqrt_spp * sqrt_spp);

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
        
        // Wrapper that selects between debug and normal views.
        RayTraceResult ray_color(const ray& r, int depth, const hittable& world, const hittable& lights) const {
            return (view_mode == 0) ? ray_color_rgb(r, depth, world, lights) 
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
        
        vec3 sample_square_stratified(int s_i, int s_j) const {
            // Returns the vector to a random point in the square sub-pixel specified by grid
            // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5]

            auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
            auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

            return vec3(px, py, 0);
        }
        
        point3 defocus_disk_sample() const {
            // returns a random point in the camera defocus disk
            auto p = random_in_unit_disk();
            return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }
        
        private:
        int image_height;           // rendered image height
        double pixel_samples_scale; // scaling factor for rgb values
        int sqrt_spp;               // square root of number of samples per pixel
        double recip_sqrt_spp;      // 1 / sqrt_spp
        point3 center;              // camera center
        point3 pixel00_loc;         // location of pixel (0, 0) 
        vec3 u, v, w;               // camera frame basis vectors
        vec3 pixel_delta_u;         // pixel spacing in horizontal
        vec3 pixel_delta_v;         // pixel spacing in vertical direction
        vec3 defocus_disk_u;        // defocus disk horizontal radius
        vec3 defocus_disk_v;        // defocus deisk vertical radius
        
        RayTraceResult ray_color_rgb(const ray& r, int depth, const hittable& world, const hittable& lights) const {
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
                accumulated_color += current_attenuation * rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);
        
                ray scattered;
                color scatter_attenuation;
                double pdf_value;

                // If the material does not scatter, we end the loop.
                if (!rec.mat->scatter(current_ray, rec, scatter_attenuation, scattered, pdf_value)) {
                    break;
                }

                auto p0 = make_shared<hittable_pdf>(lights, rec.p);
                auto p1 = make_shared<cosine_pdf>(rec.normal);
                mixture_pdf mixed_pdf(p0, p1);

                scattered = ray(rec.p, mixed_pdf.generate());             
                pdf_value = mixed_pdf.value(scattered.direction());
        
                double scattering_pdf = rec.mat->scattering_pdf(current_ray, rec, scattered);
        
                // Update the attenuation and continue with the scattered ray.
                current_attenuation *= scatter_attenuation * scattering_pdf / pdf_value;
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
                    double pdf_value;

                    if (rec.mat->scatter(current_ray, rec, attenuation, scattered, pdf_value)) {
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
            ray current_ray = r;
            hit_record rec;

            if (!world.hit(r, interval(0.001, infinity), rec))
                return RayTraceResult();
            
            // get intersection point and calculate OP. 
            // then dot with -w and compare against depth map
            // seems that rec.t = dot(OP, -w) -> not sure why
            vec3 OP = rec.p - lookfrom;
            double cam_space_z = dot(OP, -w);

            auto color_scale = 0.4 + 0.6 * std::abs(cos(rec.normal_angle));
            color ray_color = color_scale * color(1.0, 1.0, 1.0);

            return RayTraceResult(ray_color, cam_space_z);
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
