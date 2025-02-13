#pragma once

#include "vec3.h"
#include <vector>


using color = vec3;

void write_color(std::vector<uint32_t>& buffer, size_t index, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Write the translated [0,255] value of each color component
    uint8_t rbyte = static_cast<uint8_t>(255.999 * r);
    uint8_t gbyte = static_cast<uint8_t>(255.999 * g);
    uint8_t bbyte = static_cast<uint8_t>(255.999 * b);

    // Write the pixel color to the buffer
    buffer[index] = (rbyte << 24) | (gbyte << 16) | (bbyte << 8) | 0xFF;
}

void write_color(std::vector<uint32_t>& buffer, int x, int y, int width, const color& pixel_color) {
    size_t index = y * width + x;
    write_color(buffer, index, pixel_color);
}
