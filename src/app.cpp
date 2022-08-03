#include <iostream>
#include <memory>
#include <string>
#include "app.h"
#include "render_system.h"
#include "input_system.h"
#include "shader.h"
#include "log.h"
#include "model.h"
#include "texture.h"

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
  // renderpass init
  for(auto& pass : CSingleton<CRenderSystem>()->GetRenderPass()) {
    pass->Init();
  }
}

GLvoid GEngine::CApp::RunMainLoop() {
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  auto cube_shader = std::make_shared<CShader>(std::string("/Users/lance/code/GitHub/myRenderer/shaders/vert.glsl"),
                                               std::string("/Users/lance/code/GitHub/myRenderer/shaders/frag.glsl"));
  auto model_shader = std::make_shared<CShader>(std::string("/Users/lance/code/GitHub/myRenderer/shaders/model_VS.glsl"), 
                                                std::string("/Users/lance/code/GitHub/myRenderer/shaders/model_FS.glsl"));
  // texture
  std::string texture_path = "/Users/lance/code/GitHub/myRenderer/assets/textures/marble.jpg";
  auto cube_texture = std::make_shared<CTexture>(texture_path); 
  cube_shader->SetTexture("cube_texture", cube_texture);
  
  // model
  std::string model_path("/Users/lance/code/GitHub/myRenderer/assets/backpack/backpack.obj");
  auto model_backpack = GEngine::CModel(model_path);

  // render loop
  while (!glfwWindowShouldClose(window_)) {
    CalculateTime();
    CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->Tick();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ticking the render passes
    auto render_passes = CSingleton<CRenderSystem>()->GetRenderPass();
    for (int i = 0; i < render_passes.size(); i++) {
      if (render_passes[i]->GetOrder() == -1)
        continue;
      switch (render_passes[i]->GetType()) {
      case GEngine::ERenderPassType::Once:
        render_passes[i]->Tick();
        render_passes[i]->SetOrder(-1);
        break;
      default:
        render_passes[i]->Tick();
        break;
      }
    }
    // ticking the objects
    cube_shader->Use();
    CSingleton<CRenderSystem>()->RenderCube(cube_shader); // render cube
    model_shader->Use();// [render model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2, 2, 2));
    model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
    glm::mat4 view = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetViewMatrix();
    glm::mat4 projection = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetProjectionMatrix();
    glm::mat4 projection_view_model = projection * view * model;
    model_shader->SetMat4("projection_view_model", projection_view_model);
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
