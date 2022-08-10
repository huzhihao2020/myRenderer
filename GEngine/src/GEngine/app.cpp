#include <iostream>
#include <memory>
#include <string>
#include "GEngine/app.h"
#include "GEngine/render_system.h"
#include "GEngine/input_system.h"
#include "GEngine/shader.h"
#include "GEngine/log.h"
#include "GEngine/model.h"
#include "GEngine/mesh.h"
#include "GEngine/texture.h"

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
  for(auto& pass : CSingleton<CRenderSystem>()->GetRenderPass()) {
    pass->Init();
  }
}

GLvoid GEngine::CApp::RunMainLoop() {
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  auto basic_shader  = std::make_shared<CShader>(std::string("../../shaders/vert.glsl"),
                                                 std::string("../../shaders/frag.glsl"));
  auto sponza_shader = std::make_shared<CShader>(std::string("../../shaders/sponza_VS.glsl"), 
                                                 std::string("../../shaders/sponza_FS.glsl"));
  // texture
  // std::string texture_path = "../../assets/textures/marble.jpg";
  // auto marble_texture = std::make_shared<CTexture>(texture_path); 
  // basic_shader->SetTexture("diffuse_marble", marble_texture);

  // mesh
  // std::string model_path("../../assets/model/glTF/DamagedHelmet.gltf");
  // std::string model_path("../../assets/model/backpack/backpack.obj");
  std::string model_path("../../assets/model/sponza/Scale300Sponza.obj");
  // std::string model_path("../../assets/model/Lucy/Lucy.obj"); // Large model
  auto mesh_sponza = std::make_shared<GEngine::CMesh>();
  mesh_sponza->LoadMesh(model_path);

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
    basic_shader->Use();
    // CSingleton<CRenderSystem>()->RenderCube(basic_shader); // render cube
    CSingleton<CRenderSystem>()->RenderSphere(basic_shader); // render sphere
    sponza_shader->Use();// [render model
    glm::mat4 model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(2, 2, 2));
    model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
    glm::mat4 view = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetViewMatrix();
    glm::mat4 projection = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetProjectionMatrix();
    sponza_shader->SetMat4("u_model", model);
    sponza_shader->SetMat4("u_view", view);
    sponza_shader->SetMat4("u_projection", projection);
    mesh_sponza->Render(sponza_shader); // render model]

    // ticking main GUI
    CSingleton<CRenderSystem>()->GetOrCreateMainUI()->Tick();

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
