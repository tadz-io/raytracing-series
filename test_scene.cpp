#include "render.h"
#include "scenes.h"

int main() {
    hittable_list world;
    camera cam;
    
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
   
    // boxes inside the cornell box
    shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    world.add(box2);
    
    // Light Sources
    auto empty_material = shared_ptr<material>();
    quad lights(point3(343,554,332), vec3(-130,0,0), vec3(0,0,-105), empty_material);

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.samples_per_pixel = 10;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);
    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
    cam.initialize();

    // Set up the render buffer
    int image_width = cam.image_width;
    int image_height = cam.get_image_height();
    std::cout << "width: " << image_width << std::endl;
    std::cout << "height: " << image_height << std::endl;
    std::vector<uint32_t> buffer(image_width * image_height);

    // Render the scene
    cam.render(world, lights, buffer);

    // Save the rendered image to a file
    write_to_ppm(image_width, image_height, buffer, "output.ppm");

    return 0;
}

