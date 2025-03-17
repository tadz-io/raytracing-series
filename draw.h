#pragma once

#include "aabb.h"
#include "constants.h"
#include "render.h"

void draw_box_edges(
    const aabb& box, 
    const camera& cam,
    std::vector<uint32_t>& color_buffer,
    std::vector<double>& depth_buffer) {
        
    // The 8 corners of the box
    point3 corners[8] = {
        point3(box.x.min, box.y.min, box.z.min),
        point3(box.x.max, box.y.min, box.z.min),
        point3(box.x.min, box.y.max, box.z.min),
        point3(box.x.max, box.y.max, box.z.min),
        point3(box.x.min, box.y.min, box.z.max),
        point3(box.x.max, box.y.min, box.z.max),
        point3(box.x.min, box.y.max, box.z.max),
        point3(box.x.max, box.y.max, box.z.max)
    };
    
    // The 12 edges defined by pairs of corner indices
    int edges[12][2] = {
        {0, 1}, {0, 2}, {1, 3}, {2, 3}, // Bottom face
        {4, 5}, {4, 6}, {5, 7}, {6, 7}, // Top face
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Connecting edges
    };
}

void project_to_viewport(const point3& p, const camera& cam, vec3& screen_coords) {
    // vector pointing from camera origin to point p
    vec3 OP = p - cam.lookfrom;
    // project OP onto viewport basis vectors u, v and w
    auto viewport_x = dot(OP, cam.u);
    auto viewport_y = dot(OP, cam.v);
    auto viewport_z = dot(OP, cam.w);

    // map to normalized device coordinates
    auto ndc_x = viewport_x / viewport_z;
    auto ndc_y = viewport_y / viewport_y;
    // normalize ndc to [-1, 1] and shift to [0, 1]
    auto ndc_xn = (ndc_x / cam.viewport_width +  1) * 0.5;
    auto ndc_yn = (1 - ndc_y / cam.viewport_height) * 0.5; // flip Y --> check if this makes sense

    screen_coords = vec3(ndc_xn * cam.image_width, ndc_yn * cam.get_image_height(), viewport_z);

}


