#include <iostream>
#include <memory>
#include <chrono>


//OPENGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "linmath.h"

// #include "ImGuiUtils.h"

#include "baseline/ShaderProgram.h"
#include "baseline/Gui.h"
#include "baseline/Renderer.h"
#include "baseline/Scene.h"
#include "baseline/Simulation.h"

#include "baseline/GuidedFilter.h"
#include "baseline/GuidedFilterCL.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/core/ocl.hpp"
#include "opencv2/ximgproc.hpp"

// //opencl
// #define __CL_ENABLE_EXCEPTIONS
// #include <CL/cl.h>
// #include <CL/cl_intel.h>
// #include <baseline/cl.hpp>

struct Frame {
  cv::Mat left_rgb, right_rgb, left_gray, right_gray;
};

void read_images(std::string img_left_path, std::string img_right_path, Frame& frame);



static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

int main(int, char**){


    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui OpenGL3 example", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);


    //Objects
    std::shared_ptr<Scene> scene (new Scene);
    std::shared_ptr<Renderer> renderer (new Renderer);
    std::shared_ptr<Simulation> sim (new Simulation);
    renderer->init(scene);
    sim->init(scene,renderer);


    //Imgui
    ImGui_ImplGlfwGL3_Init(window, true);
    std::unique_ptr<Gui> gui (new Gui(sim));
    gui->init_fonts(); //needs to be initialized inside the context



    //Shaders
    std::shared_ptr<ShaderProgram> shader_program (new ShaderProgram);
    shader_program->initFromFiles("shaders/vert_shader.glsl", "shaders/frag_shader.glsl");
    shader_program->addUniform("MVP");
    shader_program->addAttribute("vCol");
    shader_program->addAttribute("vPos");

    //timing
    float time_avg=0;
    int times_count=1;


    std::string img_left_path="/media/alex/Data/Master/SHK/Data/middelbury/tsukuba/scene1.row3.col2.ppm";
    std::string img_right_path="/media/alex/Data/Master/SHK/Data/middelbury/tsukuba/scene1.row3.col5.ppm";
    //read images
    Frame frame;
    read_images(img_left_path, img_right_path, frame);


    //Guided filter
    int gf_radius=3;
    float gf_eps=10.0f;
    int gf_scale=4;
    // GuidedFilter guided_filter(frame.left_gray, gf_radius,gf_eps);



    while(true){
      glfwPollEvents();
      ImGui_ImplGlfwGL3_NewFrame();
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      glViewport(0, 0, width, height);


      gui->update();
      sim->update();
      renderer->draw();




      //smooth it
      //GUI GUIDED FILTER
      if (ImGui::TreeNode("GUIDED FILTER ")){
        ImGui::SliderInt("gf_radius", &gf_radius, 1, 50);
        ImGui::SliderFloat("gf_eps", &gf_eps, 0.0f, 2550.0f);
        ImGui::SliderInt("gf_scale", &gf_scale, 1, 7);
        ImGui::TreePop();
      }







      GuidedFilter guided_filter(frame.left_gray, gf_radius, gf_eps, gf_scale);
      GuidedFilterCL guided_filterCL();
      cv::Ptr<cv::ximgproc::GuidedFilter> gf_filter = cv::ximgproc::createGuidedFilter(frame.left_gray,gf_radius,gf_eps);


      cv::Mat smoothed;
      std::chrono::steady_clock::time_point begin_detect = std::chrono::steady_clock::now();
      smoothed = guided_filter.filter(frame.left_gray);
      // gf_filter->filter(frame.left_gray,smoothed,frame.left_gray.depth());
      std::chrono::steady_clock::time_point end_detect= std::chrono::steady_clock::now();


      cv::imshow("smoothed", smoothed);
      cv::waitKey(1);





      float time_f= std::chrono::duration_cast<std::chrono::nanoseconds>(end_detect - begin_detect).count() /1e6 ;
      time_avg = time_avg + (time_f - time_avg)/times_count;
      times_count++;
      ImGui::Text("Time frame (%.3f ms): ", time_f );
      ImGui::Text("Time avg (%.3f ms): ", time_avg );


      ImGui::Render();
      glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}


void read_images(std::string img_left_path, std::string img_right_path, Frame& frame){
  frame.left_rgb=cv::imread(img_left_path);
  frame.right_rgb=cv::imread(img_right_path);
  cv::cvtColor(frame.left_rgb,frame.left_gray, CV_BGR2GRAY);
  cv::cvtColor(frame.right_rgb,frame.right_gray, CV_BGR2GRAY);
}
