#pragma once

#include "constants.h"
#include "aabb.h"

class camera;

void bresenham(
    point3& p0, 
    point3& p1,
    std::vector<uint32_t>& color_buffer,
    std::vector<double>& depth_buffer,
    int image_width,
    int image_height)
    {
        // get x start and end points
        int x0 = static_cast<int>(p0.x());
        int x1 = static_cast<int>(p1.x());
        // get y start and end points
        int y0 = static_cast<int>(p0.y());
        int y1 = static_cast<int>(p1.y());
        // get z start and end points
        auto z0 = p0.z();
        auto z1 = p1.z();

        bool steep = false;
        // If the line is steep (more vertical than horizontal), 
        // swap x and y coordinates for better rasterization
        if (std::abs(x1 - x0) < std::abs(y1 - y0)) {
            std::swap(x0, y0);
            std::swap(x1, y1);
            steep = true;
        }
        
        // Ensure we draw from left to right
        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        
        int dx = x1 - x0;
        int dy = std::abs(y1 - y0);
        int error = dx / 2;
        
        int y = y0;
        int y_step = (y0 < y1) ? 1 : -1;
        
        // Loop through each x from x0 to x1
        for (int x = x0; x <= x1; x++) {
            // Interpolate z value
            double t = (dx == 0) ? 0.0 : (double)(x - x0) / dx;
            double z = z0 * (1.0 - t) + z1 * t;
            
            int pixel_x, pixel_y;
            
            // De-transpose coordinates if we're drawing a steep line
            if (steep) {
                pixel_x = y;
                pixel_y = x;
            } else {
                pixel_x = x;
                pixel_y = y;
            }
            
            // Only draw if pixel is within the image bounds
            if (pixel_x >= 0 && pixel_x < image_width && 
                pixel_y >= 0 && pixel_y < image_height) {
                    
                int index = pixel_y * image_width + pixel_x;
                if (z < depth_buffer[index])
                    write_color(color_buffer, pixel_x, pixel_y, image_width, color(1.0, 0.0, 0.0));               
            }
            
            // Update the error term and y value
            error -= dy;
            if (error < 0) {
                y += y_step;
                error += dx;
            }
        }
    }

