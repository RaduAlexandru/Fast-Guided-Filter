#include "baseline/Simulation.h"

static struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};


Simulation::Simulation(){

}


void Simulation::init (std::shared_ptr<Scene> scene, std::shared_ptr<Renderer> renderer){
  m_scene=scene;
  m_renderer=renderer;


  //Create a new Mesh and an instance for it

}

void Simulation::update (){

  //ros:spin and handle all the callbacks

  //Update camera, either with arcball or with freecam

}

// void Simulation::load_mesh(){
//   //in this case we already have it because it's given in the glfw example as a triangle
// }
//
// void Simulation::upload_mesh(){
//
// }
