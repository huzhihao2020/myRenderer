#include "skybox_pass.h"
#include "log.h"
#include "render_pass.h"
#include "render_system.h"
#include "shader.h"
#include "stb/stb_image.h"
#include "texture.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

GEngine::CSkyboxPass::CSkyboxPass(const std::string &name, int order)
    : CRenderPass(name, order) {}

GEngine::CSkyboxPass::~CSkyboxPass() {}

void GEngine::CSkyboxPass::Init() {
  std::vector<std::string> faces{
      "../../assets/textures/skybox/right.jpg",
      "../../assets/textures/skybox/left.jpg",
      "../../assets/textures/skybox/top.jpg",  
      "../../assets/textures/skybox/bottom.jpg",
      "../../assets/textures/skybox/front.jpg",
      "../../assets/textures/skybox/back.jpg"};

  std::string v_path("../../src/renderpass/skybox_shader_vert.glsl");
  std::string f_path("../../src/renderpass/skybox_shader_frag.glsl");
  shader_ = std::make_shared<GEngine::CShader>(v_path, f_path);

  skybox_texture_ = std::make_shared<GEngine::CTexture>(GEngine::CTexture::ETarget::kTextureCubeMap);
  skybox_texture_->SetMinFilter(GEngine::CTexture::EMinFilter::kLinear);
  skybox_texture_->SetMagFilter(GEngine::CTexture::EMagFilter::kLinear);
  skybox_texture_->SetSWrapMode(GEngine::CTexture::EWrapMode::kClampToEdge);
  skybox_texture_->SetTWrapMode(GEngine::CTexture::EWrapMode::kClampToEdge);
  skybox_texture_->SetRWrapMode(GEngine::CTexture::EWrapMode::kClampToEdge);
  LoadCubemapFromFiles(faces, skybox_texture_);
  shader_->Use();
  shader_->SetTexture("cubemap_texture", skybox_texture_);
}

void GEngine::CSkyboxPass::Tick() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.2, 0.3, 0.4, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  // todo
  shader_->Use();
  glm::mat4 view = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetViewMatrix();
  glm::mat4 projection = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetProjectionMatrix();
  view = glm::mat4(glm::mat3(view));
  shader_->SetMat4("view", view);
  shader_->SetMat4("projection", projection);
  CSingleton<CRenderSystem>()->RenderCube();
  glDepthFunc(GL_LESS); 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GEngine::CSkyboxPass::LoadCubemapFromFiles(const std::vector<std::string>& paths, std::shared_ptr<GEngine::CTexture> texture) {
  if(paths.size()!=6) {
    GE_ERROR("Invalid Skybox size: {0}", paths.size());
    return;
  }

  glBindTexture(GL_TEXTURE_CUBE_MAP, texture->id_);

  int width, height, nrChannels;
  for (int i = 0; i < 6; i++) {
    unsigned char *data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, static_cast<GLint>(texture->internal_format_), width, height, 0, static_cast<GLenum>(texture->external_format_), GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      GE_ERROR("Cubemap texture failed to load at path: {0}", paths[i]);
      stbi_image_free(data);
    }
  }
  texture->SetWidth(width);
  texture->SetHeight(height);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
