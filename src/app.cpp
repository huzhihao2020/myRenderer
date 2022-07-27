#include <iostream>
#include <memory>
#include "app.h"

GEngine::CApp::CApp()
{
}

GEngine::CApp::~CApp()
{
}

GLvoid GEngine::CApp::Init()
{
  std::cout << "capp init!\n";
  auto window = std::make_shared<CGLFWWindow>();
  window->Init();
  window_ = window->GetWindow();
}

GLvoid GEngine::CApp::RunMainLoop() {
  glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
while(!glfwWindowShouldClose(window_)){
  glfwPollEvents();
  glClear(GL_COLOR_BUFFER_BIT);
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
