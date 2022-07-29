#include <iostream>
#include <memory>
#include <string>
#include "app.h"
#include "render_system.h"
#include "input_system.h"
#include "shader.h"

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
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  std::string vertex_shader_path("/Users/lance/code/GitHub/myRenderer/shaders/vert.glsl");
  std::string fragment_shader_path("/Users/lance/code/GitHub/myRenderer/shaders/frag.glsl");
  auto cube_shader = GEngine::CShader(vertex_shader_path, fragment_shader_path);
  cube_shader.Use();
  while (!glfwWindowShouldClose(window_)) {
    CalculateTime();
    CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->Tick();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CSingleton<CRenderSystem>()->RenderCube(cube_shader);
    glfwSwapBuffers(window_);
    glfwPollEvents();
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
