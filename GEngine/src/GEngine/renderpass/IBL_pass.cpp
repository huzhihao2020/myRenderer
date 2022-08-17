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
  glfwMakeContextCurrent(CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow());
  // Init Irradiance Map
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  
  framebuffer_ = std::make_shared<CFrameBuffer>();
  irradiance_texture_ = std::make_shared<CTexture>(CTexture::ETarget::kTextureCubeMap);
  irradiance_texture_->SetSWrapMode(CTexture::EWrapMode::kClampToEdge),
  irradiance_texture_->SetTWrapMode(CTexture::EWrapMode::kClampToEdge),
  irradiance_texture_->SetRWrapMode(CTexture::EWrapMode::kClampToEdge),
  irradiance_texture_->SetMinFilter(CTexture::EMinFilter::kLinear);
  irradiance_texture_->SetMagFilter(CTexture::EMagFilter::kLinear);
  irradiance_texture_->SetWidth(32);
  irradiance_texture_->SetHeight(32);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_texture_->id_);
  for(unsigned int i=0; i<6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                 irradiance_texture_->GetWidth(), irradiance_texture_->GetHeight(), 
                 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, static_cast<GLint>(irradiance_texture_->GetSWrapMode()));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, static_cast<GLint>(irradiance_texture_->GetTWrapMode()));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, static_cast<GLint>(irradiance_texture_->GetRWrapMode()));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(irradiance_texture_->GetMinFilter())); 
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(irradiance_texture_->GetMagFilter()));

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  framebuffer_->SetColorAttachment(CAttachment(irradiance_texture_));
  auto renderbuffer = std::make_shared<CRenderBuffer>();
  renderbuffer->InitialzeStorage(CRenderBuffer::EPixelFormat::kDepthComponent24, 32, 32);
  framebuffer_->SetDepthAttachment(CAttachment(renderbuffer));
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    GE_WARN("Framebuffer object is not complete");
  }

  // auto skybox_cubemap = CSingleton<GEngine::CRenderSystem>()->GetAnyDataByName("skybox_cubemap");
  // auto skybox_texture = std::any_cast<std::shared_ptr<CTexture>>(skybox_cubemap);
  auto skybox_texture = CSingleton<CRenderSystem>()->texture_center_["skybox_texture"];

  // render irradiance map to off-screen framebuffer
  GenerateIrradianceMap(skybox_texture);

  // CSingleton<GEngine::CRenderSystem>()->RegisterAnyDataWithName("irradiance_cubemap", irradiance_texture_);
  CSingleton<CRenderSystem>()->texture_center_["irradiance_texture"] = irradiance_texture_;

  // Generate Prefiltered Map
  int max_mip_levels = 8;
  GeneratePrefilteredMap(skybox_texture, max_mip_levels);
  CSingleton<CRenderSystem>()->texture_center_["prefiltered_texture"] = prefiltered_texture_;

  // Load brdf_lut
  // std::string lut_path = std::string("../../assets/textures/IBL/ibl_brdf_lut.png");
  // auto sampler = std::make_shared<CSampler>(
  //     CSampler::EMinFilter::kLinear, CSampler::EMagFilter::kLinear,
  //     CSampler::EWrapMode::kClampToEdge, CSampler::EWrapMode::kClampToEdge);
  // specular_brdf_lut_ = std::make_shared<CTexture>(lut_path, CTexture::ETarget::kTexture2D, true, sampler);
  // CSingleton<CRenderSystem>()->texture_center_["ibl_brdf_lut"] = specular_brdf_lut_;
}

void GEngine::CIBLPass::Tick() {
  // do nothing
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

void GEngine::CIBLPass::GeneratePrefilteredMap(std::shared_ptr<GEngine::CTexture> skybox_texture, int max_mip_levels) {
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  // Init Prefiltered Cubemap & framebuffer
  prefiltered_texture_ = std::make_shared<CTexture>(CTexture::ETarget::kTextureCubeMap);
  prefiltered_texture_->SetSWrapMode(CTexture::EWrapMode::kClampToEdge),
  prefiltered_texture_->SetTWrapMode(CTexture::EWrapMode::kClampToEdge),
  prefiltered_texture_->SetRWrapMode(CTexture::EWrapMode::kClampToEdge),
  prefiltered_texture_->SetMinFilter(CTexture::EMinFilter::kLinearMipmapLinear);
  prefiltered_texture_->SetMagFilter(CTexture::EMagFilter::kLinear);
  prefiltered_texture_->SetWidth(256);
  prefiltered_texture_->SetHeight(256);
  prefiltered_texture_->has_mipmap_ = true;
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltered_texture_->id_);

  for(unsigned int i=0; i<6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, prefiltered_texture_->GetWidth(), prefiltered_texture_->GetHeight(), 0,
                 GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, static_cast<GLint>(prefiltered_texture_->GetSWrapMode()));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, static_cast<GLint>(prefiltered_texture_->GetTWrapMode()));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, static_cast<GLint>(prefiltered_texture_->GetRWrapMode()));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(prefiltered_texture_->GetMinFilter())); 
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(prefiltered_texture_->GetMagFilter()));
  // reserve space for lod
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


  framebuffer_ = std::make_shared<CFrameBuffer>();
  framebuffer_->SetColorAttachment(CAttachment(prefiltered_texture_));
  auto renderbuffer = std::make_shared<CRenderBuffer>();
  renderbuffer->InitialzeStorage(CRenderBuffer::EPixelFormat::kDepthComponent24, prefiltered_texture_->GetWidth(), prefiltered_texture_->GetHeight());
  framebuffer_->SetDepthAttachment(CAttachment(renderbuffer));
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      GE_WARN("Framebuffer object is not completed in IBL pass");
    }

  // render to cubemap texture
  std::string v_path("../../GEngine/src/GEngine/renderpass/ibl_irradiance_vert.glsl");
  std::string f_path("../../GEngine/src/GEngine/renderpass/ibl_prefiltered_frag.glsl");
  shader_ = std::make_shared<CShader>(v_path, f_path);

  glm::mat4 capture_projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  glm::mat4 capture_views[] = {
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
  };

  shader_->Use();
  shader_->SetTexture("cubemap_texture", skybox_texture);
  shader_->SetMat4("projection", capture_projection);

  for(unsigned int level=0; level<max_mip_levels; level++) {
    unsigned int width = static_cast<unsigned int>(prefiltered_texture_->GetWidth() * std::pow(0.5, level));
    unsigned int height = static_cast<unsigned int>(prefiltered_texture_->GetHeight() * std::pow(0.5, level));
    renderbuffer->InitialzeStorage(CRenderBuffer::EPixelFormat::kDepthComponent24, width, height);
    framebuffer_->SetDepthAttachment(CAttachment(renderbuffer));
    glViewport(0, 0, width, height);

    float roughness = (float)level / (float)(max_mip_levels - 1);
    shader_->SetFloat("roughness", roughness);
    for(unsigned int i=0; i<6; i++) {
      shader_->SetMat4("view", capture_views[i]);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefiltered_texture_->id_, level);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // glEnable(GL_DEPTH_TEST);
      // glDepthFunc(GL_LEQUAL);
      shader_->Use();
      CSingleton<GEngine::CRenderSystem>()->RenderCube();
    }
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  int screen_width, screen_height;
  auto window = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
  glfwGetFramebufferSize(window, &screen_width, &screen_height);
  glViewport(0, 0, screen_width, screen_height);
}
