#pragma once

#include "color.h"

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
