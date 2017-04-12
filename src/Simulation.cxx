#include "baseline/Simulation.h"

Simulation::Simulation(){

}


void Simulation::init (std::shared_ptr<Scene> scene, std::shared_ptr<Renderer> renderer){
  m_scene=scene;
  m_renderer=renderer;
}
