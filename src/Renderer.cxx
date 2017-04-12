#include "baseline/Renderer.h"

Renderer::Renderer(){

}

void Renderer::init (std::shared_ptr<Scene> scene){
  m_scene=scene;
}

void Renderer::draw(){
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(m_clear_color.x, m_clear_color.y, m_clear_color.z, m_clear_color.w);



}
