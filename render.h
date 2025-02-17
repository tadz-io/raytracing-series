#pragma once

#include "constants.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"

#include <fstream>

class camera {
    public:
        // public camera parameters
        double aspect_ratio;
        int image_width;
        int samples_per_pixel = 10;
        int max_depth = 5;
        point3 center = point3(0, 0, 0);    // camera center point
        double focal_length = 1.0;

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
            
            auto ray_origin = center;
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);

        }

        vec3 sample_square() const {
            // returns a vector in the [-.5,-.5] - [+.5,+.5] unit square
            return vec3(random_double() - 0.5, random_double() - 0.5, 0);
        }

    private:
        int image_height;           // rendered image height
        double pixel_samples_scale; // scaling factor for rgb values
        point3 pixel00_loc;         // location of pixel (0, 0) 
        vec3 pixel_delta_u;         // pixel spacing in horizontal
        vec3 pixel_delta_v;         // pixel spacing in vertical direction

        void initialize() {
            image_height = int(image_width / aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height;
            
            // calculate scaling factor for rgb values
            pixel_samples_scale = 1.0 / samples_per_pixel;

            // viewport parameters
            auto viewport_height = 2.0;
            auto viewport_width = viewport_height * (double(image_width)/image_height);
            auto viewport_u = vec3(viewport_width, 0, 0);
            auto viewport_v = vec3(0, -viewport_height, 0);
            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;
            auto viewport_upper_left = center 
                                        - vec3(0, 0, focal_length)
                                        - viewport_u / 2
                                        - viewport_v / 2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
        }

        color ray_color(const ray& r, int depth, const hittable& world) const {
            // if ray bounces are exceeded, no more light is gathered
            if (depth <= 0 )
                return color(0,0,0);
            hit_record rec;
            if (world.hit(r, interval(0.001, infinity), rec)){
                // return 0.5 * (rec.normal + color(1,1,1));
                // return (rec.normal_angle / pi) * color(0.1, 1, 0.2);

                vec3 direction = rec.normal + random_unit_vector();
                return 0.75 * ray_color(ray(rec.p, direction), depth-1, world);
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

