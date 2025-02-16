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
            // render and write to buffer
            for (int j = 0; j < image_height; j++) {
                for (int i = 0; i < image_width; i++) {
                    auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                    auto ray_direction = pixel_center - center;
                    ray r(center, ray_direction);
                    // color the pixel according to the ray vector
                    color pixel_color = ray_color(r, world);
                    write_color(buffer, i, j, image_width, pixel_color);
                }
            }   

        }

    private:
        int image_height;       // rendered image height
        point3 center;          // camera center
        point3 pixel00_loc;     // location of pixel (0, 0) 
        vec3 pixel_delta_u;     // pixel spacing in horizontal
        vec3 pixel_delta_v;     // pixel spacing in vertical direction

        void initialize() {
            image_height = int(image_width / aspect_ratio);
            image_height = (image_height < 1) ? 1 : image_height;
            
            // camera center point
            center = point3(0, 0, 0);          

            // viewport parameters
            auto focal_length = 1.0;
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

        color ray_color(const ray& r, const hittable& world) const {
            hit_record rec;
            if (world.hit(r, interval(0, infinity), rec)){
                return 0.5 * (rec.normal + color(1,1,1));
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

