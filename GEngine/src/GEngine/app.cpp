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
#include "singleton.h"

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
  auto pbr_shader = std::make_shared<CShader>(std::string("../../shaders/PBR_MR_VS.glsl"),
                                              std::string("../../shaders/PBR_MR_FS.glsl"));
  auto sponza_obj_shader = std::make_shared<CShader>(std::string("../../shaders/sponza_VS.glsl"), 
                                                 std::string("../../shaders/sponza_FS.glsl"));
  auto sponza_pbr_shader = std::make_shared<CShader>(std::string("../../shaders/sponza_PBR_VS.glsl"),
                                                     std::string("../../shaders/sponza_PBR_FS.glsl"));
  auto sponza_pbr_shader_debug = std::make_shared<CShader>(std::string("../../shaders/sponza_PBR_VS_debug.glsl"),
                                                           std::string("../../shaders/sponza_PBR_FS_debug.glsl"));
  auto light_shader = std::make_shared<CShader>(std::string("../../shaders/light_sphere_VS.glsl"),
                                                std::string("../../shaders/light_sphere_FS.glsl"));

  bool render_light        = 0;
  bool render_sponza_phong = 0;
  bool render_sponza_pbr   = 0;
  bool render_pbr_sphere   = 1;

  // texture
  if(render_light) {
    std::string texture_path = "../../assets/textures/marble.jpg";
    auto marble_texture = std::make_shared<CTexture>(texture_path); 
    basic_shader->SetTexture("diffuse_marble", marble_texture);
  }

  // mesh
  // std::string model_path("../../assets/model/glTF/DamagedHelmet.gltf");
  // std::string model_path("../../assets/model/backpack/backpack.obj");
  // std::string model_path("../../assets/model/Lucy/Lucy.obj"); // Large model
  auto mesh_sponza = std::make_shared<GEngine::CMesh>();
  if(render_sponza_phong) {
    mesh_sponza->LoadMesh(std::string("../../assets/model/sponza/Scale300Sponza.obj"));
  } else if(render_sponza_pbr) {
    mesh_sponza->LoadMesh(std::string("../../assets/model/sponza-gltf-pbr/sponza.glb"));
  }

  // sphere_pbr_ibl
  // auto ibl_irradiance_data = CSingleton<CRenderSystem>()->GetAnyDataByName("irradiance_cubemap");
  // auto ibl_irradiance_texture = std::any_cast<std::shared_ptr<CTexture>>(ibl_irradiance_data);

  if (render_pbr_sphere) {
    auto ibl_irradiance_texture = CSingleton<CRenderSystem>()->texture_center_["irradiance_texture"];
    auto ibl_prefiltered_texture = CSingleton<CRenderSystem>()->texture_center_["prefiltered_texture"];
    auto ibl_brdf_lut = CSingleton<CRenderSystem>()->texture_center_["ibl_brdf_lut"];
    // auto ibl_irradiance_texture = CSingleton<CRenderSystem>()->texture_center_["skybox_texture"];
    if (!ibl_irradiance_texture) {
      GE_WARN("ibl_irradiance_texture not exists!");
    }
    if (!ibl_prefiltered_texture) {
      GE_WARN("ibl_prefiltered_texture not exists!");
    }
    if (!ibl_brdf_lut) {
      GE_WARN("ibl_brdf_lut not exists!");
    }
    pbr_shader->SetTexture("ibl_irradiance_cubemap", ibl_irradiance_texture);
    pbr_shader->SetTexture("ibl_prefilter_map", ibl_prefiltered_texture);
    pbr_shader->SetTexture("ibl_brdf_lut", ibl_brdf_lut);
  }

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
    glm::mat4 view = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetViewMatrix();
    glm::mat4 projection = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetProjectionMatrix();
    glm::mat4 projection_view_model = projection * view * model;
    if(render_light) {
      basic_shader->Use();
      basic_shader->SetMat4("projection_view_model", projection_view_model);
      CSingleton<CRenderSystem>()->RenderSphere(); // render sphere
    }   
    // render model
    if (render_sponza_phong) {
      sponza_obj_shader->Use();
      model = glm::mat4(1.0f);
      // model = glm::scale(model, glm::vec3(10.0, 10.0, 10.0)); 
      model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
      sponza_obj_shader->SetMat4("u_model", model);
      sponza_obj_shader->SetMat4("u_view", view);
      sponza_obj_shader->SetMat4("u_projection", projection);
      mesh_sponza->Render(sponza_obj_shader); // render model]
    } else if (render_sponza_pbr) {
      auto sponza_shader = sponza_pbr_shader;

      if(CSingleton<CRenderSystem>()->GetOrCreateMainUI()->test_button_status_) {
        sponza_shader = sponza_pbr_shader_debug;
      }
      int window_size[2];
      glfwGetWindowSize(window_, &window_size[0], &window_size[1]);
      sponza_shader->Use();
      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0, -50.0, 0.0));
      model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
      sponza_shader->SetMat4("u_model", model);
      sponza_shader->SetMat4("u_view", view);
      sponza_shader->SetMat4("u_projection", projection);
      sponza_shader->SetVec3("u_basecolor", glm::vec3(0.5, 0.0, 0.0));
      sponza_shader->SetFloat("u_metallic", 0.5);
      sponza_shader->SetFloat("u_roughness", 0.5);
      sponza_shader->SetVec2("u_viewport_size", glm::vec2(window_size[0], window_size[1]));
      mesh_sponza->Render(sponza_shader);
    }

    if(render_pbr_sphere) {
      pbr_shader->Use();
      pbr_shader->SetMat4("u_view", view);
      pbr_shader->SetMat4("u_projection", projection);
      int nrRows = 7;
      int nrColumns = 7;
      float spacing = 2.5;
      pbr_shader->SetVec3("u_basecolor", glm::vec3(0.5, 0.0, 0.0));
      for (int row = 0; row < nrRows; ++row) {
        pbr_shader->SetFloat("u_metallic", (float)row / (float)nrRows);
        for (int col = 0; col < nrColumns; ++col) {
          pbr_shader->SetFloat("u_roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));
          model = glm::mat4(1.0f);
          model = glm::translate(model, glm::vec3((float)(col - (nrColumns / 2)) * spacing, (float)(row - (nrRows / 2)) * spacing, 0.0f));
          pbr_shader->SetMat4("u_model", model);
          CSingleton<CRenderSystem>()->RenderSphere();
        }
      }
      float x = 7.5f;
      std::vector<glm::vec3> lights_pos = {
        glm::vec3(-x,  x, 10.0),
        glm::vec3(-x, -x, 10.0),
        glm::vec3( x,  x, 10.0),
        glm::vec3( x, -x, 10.0),
      };
      light_shader->Use();
      light_shader->SetMat4("u_view", view);
      light_shader->SetMat4("u_projection", projection);
      for(int i=0; i<4; i++) {
          model = glm::mat4(1.0f);
          model = glm::translate(model, lights_pos[i]);
          model = glm::scale(model, glm::vec3(0.5f));
          light_shader->SetMat4("u_model", model);
          CSingleton<CRenderSystem>()->RenderSphere();
      }
    }

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
