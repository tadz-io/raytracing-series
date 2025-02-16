#pragma once

#include "vec3.h"
#include "constants.h"
#include <vector>

using color = vec3;

void write_color(std::vector<uint32_t>& buffer, size_t index, const color& pixel_color) {
    // Convert from [0,1] float to [0,255] integer
    uint8_t rbyte = static_cast<uint8_t>(255.999 * pixel_color.x());
    uint8_t gbyte = static_cast<uint8_t>(255.999 * pixel_color.y());
    uint8_t bbyte = static_cast<uint8_t>(255.999 * pixel_color.z());

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
        std::cout << "buffer write: R=" << r << " G=" << g << " B=" << b << std::endl;
        std::cout << "Packed value: 0x" << std::hex << buffer[index] << std::dec << std::endl;
    }
}
