#pragma once

#include "color.h"
#include <fstream>

int height = 256;
int width = 256;

void render(std::vector<uint32_t>& buffer) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            color pixel_color = color(double(i)/(width-1), double(j)/(height-1), 0);
            write_color(buffer, i, j, width, pixel_color);
        }
    }
}

void write_to_ppm(const std::vector<uint32_t>& buffer, const std::string& filename) {
    std::ofstream out(filename);
    out << "P3\n" << width << ' ' << height << "\n255\n";

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            uint32_t pixel = buffer[j * width + i];
            
            // Extract from ABGR
            int b = (pixel >> 16) & 0xFF;
            int g = (pixel >> 8)  & 0xFF;
            int r =  pixel        & 0xFF;
            
            if (i == 126 && j == 126) {  // Print first pixel only to avoid spam
                std::cout << "PPM write: R=" << r << " G=" << g << " B=" << b << std::endl;
                std::cout << "From value: 0x" << std::hex << pixel << std::dec << std::endl;
            }
            
            // Write as RGB (PPM format)
            out << r << ' ' << g << ' ' << b << '\n';
        }
    }
    out.close();
}

