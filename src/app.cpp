#include <iostream>
#include <memory>
#include "app.h"
#include "render_system.h"
#include "input_system.h"

GEngine::CApp::CApp()
{
}

GEngine::CApp::~CApp()
{
}

GLvoid GEngine::CApp::Init()
{
  std::cout << "CApp init!\n";
  CSingleton<CRenderSystem>()->GetOrCreateWindow()->Init();
  window_ = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
  CSingleton<CRenderSystem>()->Init();
  CSingleton<CInputSystem>()->Init();
}

GLvoid GEngine::CApp::RunMainLoop() {
  glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
  while (!glfwWindowShouldClose(window_)) {
    CalculateTime();
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();
    glfwSwapBuffers(window_);
  }
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
