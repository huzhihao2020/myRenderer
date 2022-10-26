#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "GEngine/texture.h"

namespace GEngine {
class CMaterial {
  enum class MATERIAL_TYPE {
    Phong,
    PBR_MetallicRoughness,
    Principled_BSDF,
  };

  struct SMaterialDesc {
    // todo 
    bool has_base_color = false;
    bool has_base_color_texture = false;
  };

public:
  MATERIAL_TYPE material_type_ = MATERIAL_TYPE::PBR_MetallicRoughness;
  SMaterialDesc mat_desc_;

  //Common
  std::shared_ptr<CTexture> diffuse_texture_ = nullptr;
  std::shared_ptr<CTexture> normal_texture_ = nullptr;
  std::shared_ptr<CTexture> alpha_texture_ = nullptr;

  // Phong
  float shiness = 32.0f;

  // PBR Metallic-Roughness
  glm::vec3 basecolor_ = glm::vec3(0.5f, 0.0f, 0.0f);
  glm::vec3 emissive_  = glm::vec3(0.0f, 0.0f, 0.0f);
  float default_f0_ = 0.04f;
  float default_roughness_ = 0.5f;
  float default_metallic_ = 0.0f;
  float default_ao_ = 0.0f;
  std::shared_ptr<CTexture> basecolor_texture_ = nullptr;
  std::shared_ptr<CTexture> roughness_texture_ = nullptr;
  std::shared_ptr<CTexture> metallic_texture_  = nullptr;
  std::shared_ptr<CTexture> ao_texture_        = nullptr;
  std::shared_ptr<CTexture> emissive_texture_  = nullptr;
  std::shared_ptr<CTexture> unknown_texture_   = nullptr;

  // todo: Disney Principled BSDF

};
} // namespace GEngine