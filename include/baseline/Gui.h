#pragma once

#include <iostream>
#include <memory>

#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include "curve.hpp"
#include "IconsFontAwesome.h"

#include "baseline/Simulation.h"

class Gui{
public:
  // Gui();
  Gui(std::shared_ptr<Simulation> sim );
  void update();

  //Needs to be inline otherwise it doesn't work since it needs to be executed from a GL context
  inline void init_fonts(){
    ImGui::GetIO().Fonts->AddFontFromFileTTF("./deps/imgui/extra_fonts/Roboto-Regular.ttf", 14.0f);
    ImFontConfig config;
    config.MergeMode = true;
    const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImGui::GetIO().Fonts->AddFontFromFileTTF("./deps/imgui/extra_fonts/fontawesome-webfont.ttf", 13.0f, &config, icon_ranges);
  };
  void init_fonts2(ImGuiIO& io);
  void init_style();
private:
  // ImGuiIO& io= ImGui::GetIO();
  std::shared_ptr<Simulation> m_sim;

  bool m_show_demo_window;

  ImVec2 foo[10];





};
