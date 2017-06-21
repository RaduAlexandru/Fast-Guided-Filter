#pragma once

#include <memory>

#include "baseline/Renderer.h"
#include "baseline/Scene.h"

class Simulation{
public:
  Simulation();

  std::shared_ptr<Renderer> m_renderer;
  std::shared_ptr<Scene> m_scene;

  void init (std::shared_ptr<Scene> scene, std::shared_ptr<Renderer> renderer);
  void update ();
private:

};
