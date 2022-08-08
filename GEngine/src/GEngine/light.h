#pragma once
#include <memory>
#include <glm/glm.hpp>

namespace GEngine {
class CLight {
public:
  enum class LightType { Ambient, Omni, Spot, Directional };

  CLight();
  CLight(LightType type);
  ~CLight();

  // common
  LightType type_ = LightType::Omni;
  glm::vec3 intensity_ = glm::vec3(0.5f);
  glm::vec3 position_  = glm::vec3(0.0f);
  glm::vec3 direction_ = glm::vec3(0.0f, -1.0f, 0.0f);
  // Omni
  float radius_ = 10.0f;
  // Spot
  float inner_angle = 20.0f;
  float outer_angle = 45.0f;

private:
};
} // namespace GEngine
