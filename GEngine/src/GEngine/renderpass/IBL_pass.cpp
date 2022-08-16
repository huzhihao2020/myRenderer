#include "GEngine/renderpass/IBL_pass.h"
#include "GEngine/singleton.h"
#include "GEngine/render_system.h"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "log.h"
#include <memory>

GEngine::CIBLPass::CIBLPass(const std::string &name, int order)
    : CRenderPass(name, order) {}

GEngine::CIBLPass::~CIBLPass() {}

void GEngine::CIBLPass::Init() {
  // todo
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  framebuffer_ = std::make_shared<CFrameBuffer>();
  irradiance_texture_ = std::make_shared<CTexture>(CTexture::ETarget::kTextureCubeMap);
  irradiance_texture_->SetWidth(32);
  irradiance_texture_->SetHeight(32);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_texture_->id_);
  // init irradiance map
  for(unsigned int i=0; i<6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, static_cast<GLuint>(irradiance_texture_->GetSWrapMode()));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, static_cast<GLuint>(irradiance_texture_->GetTWrapMode()));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, static_cast<GLuint>(irradiance_texture_->GetRWrapMode()));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, static_cast<GLuint>(irradiance_texture_->GetMinFilter())); 
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, static_cast<GLuint>(irradiance_texture_->GetMagFilter()));

  framebuffer_->SetColorAttachment(CAttachment(irradiance_texture_));
  auto renderbuffer = std::make_shared<CRenderBuffer>();
  renderbuffer->InitialzeStorage(CRenderBuffer::EPixelFormat::kDepthComponent24, 32, 32);
  framebuffer_->SetDepthAttachment(CAttachment(renderbuffer));
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    GE_WARN("Framebuffer object is not complete");
  }

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  // auto skybox_cubemap = CSingleton<GEngine::CRenderSystem>()->GetAnyDataByName("skybox_cubemap");
  // auto skybox_texture = std::any_cast<std::shared_ptr<CTexture>>(skybox_cubemap);
  auto skybox_texture = CSingleton<CRenderSystem>()->texture_center_["skybox_texture"];

  // render irradiance map to off-screen framebuffer
  GenerateIrradianceMap(skybox_texture);

  // CSingleton<GEngine::CRenderSystem>()->RegisterAnyDataWithName("irradiance_cubemap", irradiance_texture_);
  CSingleton<CRenderSystem>()->texture_center_["irradiance_texture"] = irradiance_texture_;

  // test - display irradiance map
  // std::string v_path("../../GEngine/src/GEngine/renderpass/skybox_shader_vert.glsl");
  // std::string f_path("../../GEngine/src/GEngine/renderpass/skybox_shader_frag.glsl");
  // shader_ = std::make_shared<GEngine::CShader>(v_path, f_path);
  // auto ibl_irradiance = CSingleton<CRenderSystem>()->GetAnyDataByName("irradiance_cubemap");
  // auto ibl_irradiance_texture = std::any_cast<std::shared_ptr<CTexture>>(ibl_irradiance);
  // shader_->SetTexture("ibl_irradiance_cubemap", irradiance_texture_);
}

void GEngine::CIBLPass::Tick() {
  // test - display irradiance map
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glClearColor(0.2, 0.3, 0.4, 1.0);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // glEnable(GL_DEPTH_TEST);
  // glDepthFunc(GL_LEQUAL);
  // shader_->Use();
  // glm::mat4 view = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetViewMatrix();
  // glm::mat4 projection = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetProjectionMatrix();
  // view = glm::mat4(glm::mat3(view));
  // shader_->SetMat4("view", view);
  // shader_->SetMat4("projection", projection);
  // CSingleton<CRenderSystem>()->RenderCube();
  // glDepthFunc(GL_LESS); 
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// create an irradiance cubemap, stores in a FBO
void GEngine::CIBLPass::GenerateIrradianceMap(std::shared_ptr<GEngine::CTexture> skybox_texture) {
  glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  glm::mat4 capture_views[] = {
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
  };
  std::string v_path("../../GEngine/src/GEngine/renderpass/ibl_irradiance_vert.glsl");
  std::string f_path("../../GEngine/src/GEngine/renderpass/ibl_irradiance_frag.glsl");
  shader_ = std::make_shared<CShader>(v_path, f_path);
  shader_->SetTexture("cubemap_texture", skybox_texture);
  // render irradiance map to off-screen framebuffer
  shader_->Use();
  shader_->SetMat4("projection", capture_projection);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_->GetID());
  glViewport(0, 0, 32, 32);
  for(unsigned int i=0; i<6; i++) {
    shader_->SetMat4("view", capture_views[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_texture_->id_, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CSingleton<GEngine::CRenderSystem>()->RenderCube();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  int screen_width, screen_height; 
  auto window = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
  glfwGetFramebufferSize(window, &screen_width, &screen_height);
  glViewport(0, 0, screen_width, screen_height);
}
