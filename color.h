#pragma once

#include "interval.h"
#include "vec3.h"
#include "constants.h"
#include <vector>

using color = vec3;

void write_color(std::vector<uint32_t>& buffer, size_t index, const color& pixel_color) {
    // get rgb values
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // clamp values to byte range [0, 255]
    static const interval intensity(0.000, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));


    // Print original color values
    // std::cout << "Original color: R=" << (int)rbyte << " G=" << (int)gbyte << " B=" << (int)bbyte << std::endl;

    // Store as ABGR for OpenGL texture
    buffer[index] = (0xFF << 24) | (bbyte << 16) | (gbyte << 8) | rbyte;
}

void write_color(std::vector<uint32_t>& buffer, int x, int y, int width, const color& pixel_color) {
    size_t index = y * width + x;
    write_color(buffer, index, pixel_color);
    if (x == 126 && y == 126) {
        // Print packed value
        // Extract from ABGR
        int b = (buffer[index] >> 16) & 0xFF;
        int g = (buffer[index] >> 8)  & 0xFF;
        int r =  buffer[index]        & 0xFF;
        // std::cout << "buffer write: R=" << r << " G=" << g << " B=" << b << std::endl;
        // std::cout << "Packed value: 0x" << std::hex << buffer[index] << std::dec << std::endl;
    }
}
