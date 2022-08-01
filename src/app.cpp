#include <iostream>
#include <memory>
#include <string>
#include "app.h"
#include "render_system.h"
#include "input_system.h"
#include "shader.h"
#include "log.h"
#include "model.h"

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
  CSingleton<CRenderSystem>()->Init();
  CSingleton<CInputSystem>()->Init();
}

GLvoid GEngine::CApp::RunMainLoop() {
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  // shader
  std::string vertex_shader_path1("/Users/lance/code/GitHub/myRenderer/shaders/vert.glsl");
  std::string fragment_shader_path1("/Users/lance/code/GitHub/myRenderer/shaders/frag.glsl");
  auto cube_shader = GEngine::CShader(vertex_shader_path1, fragment_shader_path1);
  std::string vertex_shader_path2("/Users/lance/code/GitHub/myRenderer/shaders/model_VS.glsl");
  std::string fragment_shader_path2("/Users/lance/code/GitHub/myRenderer/shaders/model_FS.glsl");
  auto model_shader = GEngine::CShader(vertex_shader_path2, fragment_shader_path2);
  // texture
  std::string texture_path = "/Users/lance/code/GitHub/myRenderer/assets/images/marble.jpg";
  auto cube_texture = CSingleton<CRenderSystem>()->LoadTexture(texture_path); 
  cube_shader.Use();
  cube_shader.SetInt("cube_texture", 0);
  // model
  std::string model_path("/Users/lance/code/GitHub/myRenderer/assets/backpack/backpack.obj");
  auto model_backpack = GEngine::CModel(model_path);
  model_shader.Use();
  // render loop
  while (!glfwWindowShouldClose(window_)) {
    CalculateTime();
    CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->Tick();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CSingleton<CRenderSystem>()->RenderCube(cube_shader); // render cube
    model_shader.Use();// [render model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2, 2, 2));
    model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
    glm::mat4 view = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetViewMatrix();
    glm::mat4 projection = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetProjectionMatrix();
    glm::mat4 projection_view_model = projection * view * model;
    model_shader.SetMat4("projection_view_model", projection_view_model);
    model_backpack.Draw(model_shader); // render model]
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
