#pragma once

#include "constants.h"
#include "render.h"
#include "hittable_list.h"
#include "material.h"
#include "quad.h"



void sandbox(camera& cam, hittable_list& world) {
    auto difflight = make_shared<diffuse_light>(color(4.0,4.0,4.0));
    auto checker         = make_shared<checker_texture>(1.0, color(.01, .01, .01), color(.9, .9, .9));
    auto material_ground = make_shared<lambertian>(checker);
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left   = make_shared<dielectric>(1.50);
    auto material_bubble = make_shared<dielectric>(1.00 / 1.50);
    auto material_right  = make_shared<metal>(color(0.8, 0.8, 0.8), 0.0);
    
    world.add(make_shared<sphere>(point3( 0.0, -1000.5, -1.0), 1000.0, material_ground));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -8.0),   0.5, material_left));
    world.add(make_shared<sphere>(point3( 0.0,    0.0, -2.0),   0.5, material_center));
    // world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.4, material_bubble));
    world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));
    world.add(make_shared<disk>  (point3( 0.0,    2.0,  -1.0), vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), difflight));

    cam.aspect_ratio =  16.0 / 9.0;
    cam.lookat = point3(0, 0, 0);
    cam.lookfrom = point3(0, 0, 2);
    cam.image_width  = 800;
    cam.initialize();
}

void cornell_box(camera& cam, hittable_list& world) {
    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    world.add(box(point3(130, 0, 65), point3(295, 165, 230), white));
    world.add(box(point3(265, 0, 295), point3(430, 330, 460), white));

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.initialize();

    cam.samples_per_pixel = 2;
    cam.max_depth         = 2;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
}
