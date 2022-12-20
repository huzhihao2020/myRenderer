#include <iostream>
#include <memory>
#include <string>
#include "GEngine/app.h"
#include "GEngine/render_system.h"
#include "GEngine/input_system.h"
#include "GEngine/shader.h"
#include "GEngine/log.h"
#include "GEngine/mesh.h"
#include "GEngine/texture.h"
#include "glm/ext/matrix_transform.hpp"
#include "singleton.h"
#include "GEngine/animator.h"

#include <glm/gtx/string_cast.hpp>

GEngine::CApp::CApp()
{
}

GEngine::CApp::~CApp()
{
}

GLvoid GEngine::CApp::Init()
{
  
  GEngine::CLog::Init();
  CSingleton<CRenderSystem>()->GetOrCreateWindow()->Init();
  window_ = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
  CSingleton<CInputSystem>()->Init(); 
  CSingleton<CRenderSystem>()->Init();

  // renderpass init
  auto passes = CSingleton<CRenderSystem>()->GetRenderPass();
  for (int i = 0; i < passes.size(); i++) {
    passes[i]->Init();
  }
}

GLvoid GEngine::CApp::RunMainLoop() {

  // draw in wireframe
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // render loop
  while (!glfwWindowShouldClose(window_)) {
    // fixme
    CSingleton<CRenderSystem>()->GetOrCreateWindow()->SetViewport();
    
    CalculateTime();
    CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->Tick();
    glClearColor(0.2f, 0.3f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ticking the render passes
    auto render_passes = CSingleton<CRenderSystem>()->GetRenderPass();
    for (int i = 0; i < render_passes.size(); i++) {
      if (render_passes[i]->GetOrder() == -1)
        continue;
      switch (render_passes[i]->GetType()) {
      case GEngine::CRenderPass::ERenderPassType::Once:
        render_passes[i]->Tick();
        render_passes[i]->SetOrder(-1);
        break;
      default:
        render_passes[i]->Tick();
        break;
      }
    }
    
    // render the objects
    glm::mat4 model = glm::mat4(1.0f);
    // model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
    model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
    glm::mat4 view = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetViewMatrix();
    glm::mat4 projection = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetProjectionMatrix();
    glm::mat4 projection_view_model = projection * view * model;


    // ticking main GUI
    CSingleton<CRenderSystem>()->GetOrCreateMainUI()->Tick();
    
    auto err = glGetError();
    if(err!=GL_NO_ERROR) {
      GE_WARN("gl Error {0}", err);
    }
    
    glfwPollEvents();
    glfwSwapBuffers(window_);
  }
  glfwTerminate();
}

void GEngine::CApp::CalculateTime() {
  currentTime_ = glfwGetTime();
  deltaTime_ = currentTime_ - lastFrameTime_;
  lastFrameTime_ = currentTime_;

  ++frameCounter_;
  deltaTimeCounter_ += deltaTime_;
  if(deltaTimeCounter_ > 1.0) {
    currentFPS_ = frameCounter_;
    frameCounter_ = 0;
    deltaTimeCounter_ = 0.0;
  }
}

GLdouble GEngine::CApp::GetCurrentTime() const {
  return currentTime_;
}

GLdouble GEngine::CApp::GetDeltaTime() const {
  return deltaTime_;
}

GLuint GEngine::CApp::GetFPS() const {
  return currentFPS_;
}
