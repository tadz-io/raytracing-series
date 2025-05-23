#pragma once

#include "constants.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"

#include <fstream>

class camera {
    public:
        // public camera parameters
        double aspect_ratio;
        int image_width;
        int samples_per_pixel = 1;
        int max_depth = 2;
        
        double vfov = 20;                   // vertical field of view
        point3 lookfrom = point3(1,1,3);    // point camera is looking from
        point3 lookat = point3(0,0,-1);     // point camera is looking at
        vec3 vup = vec3(0,1,0);             // camera-relative up direction

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
            
            // render and write to buffer
            for (int j = 0; j < image_height; j++) {
                for (int i = 0; i < image_width; i++) {
                    // auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                    // auto ray_direction = pixel_center - center;
                    // ray r(center, ray_direction);
                    // // color the pixel according to the ray vector
                    // color pixel_color = ray_color(r, world);
                    // write_color(buffer, i, j, image_width, pixel_color);

                    color pixel_color(0,0,0);
                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                    write_color(buffer, i, j, image_width, pixel_samples_scale * pixel_color);
                }
            }   

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
        vec3 pixel_delta_u;         // pixel spacing in horizontal
        vec3 pixel_delta_v;         // pixel spacing in vertical direction
        vec3 u, v, w;               // camera frame basis vectors
        vec3 defocus_disk_u;        // defocus disk horizontal radius
        vec3 defocus_disk_v;        // defocus deisk vertical radius

        void initialize() {
            image_height = int(image_width / aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height;
            
            // calculate scaling factor for rgb values
            pixel_samples_scale = 1.0 / samples_per_pixel;

            center = lookfrom;

            // viewport parameters
            auto theta = degrees_to_radians(vfov);
            auto viewport_height = 2 * std::tan(theta/2) * focus_dist;
            auto viewport_width = viewport_height * (double(image_width)/image_height);
            
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

        color ray_color(const ray& r, int depth, const hittable& world) const {
            // if ray bounces are exceeded, no more light is gathered
            if (depth <= 0 )
                return color(0,0,0);
            
            hit_record rec;
            
            if (world.hit(r, interval(0.001, infinity), rec)){
                ray scattered;
                color attenuation;
                if (rec.mat->scatter(r, rec, attenuation, scattered))
                    return attenuation * ray_color(scattered, depth-1, world);
                return color(0,0,0);
            }
            
            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5 * (unit_direction.y() + 1.0);
            return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
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

