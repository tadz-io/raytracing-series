#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#define GL_SILENCE_DEPRECATION // To silence deprecation warnings
#include <GLFW/glfw3.h>
#include "render.h"

int main(int, char**)
{
    hittable_list world;


    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left   = make_shared<dielectric>(1.50);
    auto material_bubble = make_shared<dielectric>(1.00 / 1.50);
    auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);

    world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.2),   0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.4, material_bubble));
    world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));

    double aspect_ratio {16.0 / 9.0};
    int image_width {400};
    
    // setup camera model
    camera cam(aspect_ratio, image_width);
    cam.defocus_angle = 0.6;
    cam.focus_dist    = 3.4;
    // get image height
    int image_height {cam.get_image_height()};  
    // create buffer to write rendered image to
    std::vector<uint32_t> buffer(image_width * image_height);

    // camera look from position
    point3 lookfrom = point3(13,2,3);
    // focal length
    double vfov = 20;
    // sampling
    int samples_per_pixel = 2;
    int max_depth = 2;
    // Setup window
    if (!glfwInit())
        return -1;

    // GL
    const char* glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui Example", NULL, NULL);
    if (window == NULL)
        return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events
        glfwPollEvents();

        // Define a small step value for camera movement.
        const double step = 0.1;
        bool updated = false;

        // Check arrow keys and update camera center coordinates.
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            lookfrom = lookfrom - point3(step, 0, 0);
            updated = true;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            lookfrom = lookfrom + point3(step, 0, 0);
            updated = true;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            lookfrom = lookfrom + point3(0, step, 0);
            updated = true;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            lookfrom = lookfrom - point3(0, step, 0);
            updated = true;
        }

        // If any arrow key was pressed, update the camera and re-render.
        if (updated) {
            cam.lookfrom = lookfrom;
            cam.render(world, buffer);
            // Update texture with new render
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_width, image_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        }

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Settings");
        // Input text boxes for camera center coordinates
        if (
            ImGui::InputDouble("x: ", &lookfrom[0]) || 
            ImGui::InputDouble("y: ", &lookfrom[1]) || 
            ImGui::InputDouble("z: ", &lookfrom[2]) ||
            ImGui::InputDouble("fov: ", &vfov) ||
            ImGui::InputInt("samples per pixel: ", &samples_per_pixel) ||
            ImGui::InputInt("max depth: ", &max_depth)
        ){
            cam.lookfrom = lookfrom;
            cam.vfov = vfov;
            cam.samples_per_pixel = samples_per_pixel;
            cam.max_depth = max_depth;

            cam.render(world, buffer);
            // write_to_ppm(image_width, image_height, buffer, "render.ppm");
            // Update texture with new render
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_width, image_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        }
        ImGui::End();

        
        ImGui::Begin("Render");

        if (ImGui::Button("Render")) {
            cam.render(world, buffer);
            //write_to_ppm(image_width, image_height, buffer, "render.ppm");
            // Update texture with new render
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_width, image_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        }
        ImGui::Image((ImTextureID)textureID, ImVec2(image_width, image_height));
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glDeleteTextures(1, &textureID);
    glfwTerminate();

    return 0;
}
