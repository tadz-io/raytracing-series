#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#define GL_SILENCE_DEPRECATION // To silence deprecation warnings

#include <GLFW/glfw3.h>
#include "render.h"
#include "bvh.h"
#include "quad.h"
#include "scenes.h"

// Define function type for scene setup
typedef void (*SceneSetupFunc)(camera&, hittable_list&);

// Scene options struct
struct SceneOption {
    const char* name;
    SceneSetupFunc setup;
};

int main(int, char**)
{
    // Create an array of scene functions and their names
    SceneOption scenes[] = {
        {"Sand Box", sandbox},
        {"Cornell Box", cornell_box},
    };
    int current_scene = 0;  // Start with Cornell Box

    hittable_list world;
   
    camera cam;
    // Initialize with first scene
    scenes[current_scene].setup(cam, world);
    // sget image height
    int image_height = cam.get_image_height();  
    int image_width = cam.image_width;

    // create buffer to write rendered image to
    std::vector<uint32_t> buffer(image_width * image_height);
    // camera look from position
    point3 lookfrom = cam.lookfrom;
    // focal length
    double vfov = cam.vfov;
    // sampling
    int samples_per_pixel = cam.samples_per_pixel;
    int max_depth = cam.max_depth;
    
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

    bool update_render = true;
    
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
            update_render = true;
        }

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Settings");
        
        // Scene selection dropdown
        if (ImGui::Combo("Scene", &current_scene, 
            [](void* data, int idx, const char** out_text) {
                SceneOption* scenes = (SceneOption*)data;
                *out_text = scenes[idx].name;
                return true;
            }, 
            scenes, IM_ARRAYSIZE(scenes))) 
        {

            // Clear the current world
            world = hittable_list();
            
            // Load the selected scene
            scenes[current_scene].setup(cam, world);
            
            // Update local variables with new camera settings
            lookfrom = cam.lookfrom;
            vfov = cam.vfov;
            samples_per_pixel = cam.samples_per_pixel;
            max_depth = cam.max_depth;
            image_height = cam.get_image_height();
            image_width = cam.image_width;
            

            buffer.resize(image_width * image_height);                
            // Update OpenGL texture size
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            update_render = true;
        }
        
        // toggle between rgb render and debug mode
        const char* render_modes[] = { "Normal", "Debug", "BVH"};
        static int current_mode = 0;  // 0 for normal, 1 for debug
        if (ImGui::Combo("Render Mode", &current_mode, render_modes, IM_ARRAYSIZE(render_modes))) {
            cam.view_mode = current_mode;
            update_render = true;
        }
        
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
            update_render = true;
        }
        ImGui::End();

        // render scene when updated
        if (update_render) {
            cam.render(world, buffer);
            // Update texture with new render
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_width, image_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

            update_render = false; // Reset the flag
        }
        
        ImGui::Begin("Render");
        ImGui::Image((ImTextureID)textureID, ImVec2(image_width, image_height));
        ImGui::End();

        // Display render time and FPS in a small rectangle box in the top right corner
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 150, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(150, 50), ImGuiCond_Always);
        ImGui::Begin("Render Info", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("Render time: %.0f ms", 1000 / cam.fps);
        ImGui::Text("FPS: %.1f", cam.fps);
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
