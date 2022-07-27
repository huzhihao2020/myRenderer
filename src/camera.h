#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <vector>

namespace GEngine {

class CCamera {
public:
  enum class ECameraMovement { Forward, Backward, Left, Right };
  // camera Attributes
  glm::vec3 position_;
  glm::vec3 front_;
  glm::vec3 up_;
  glm::vec3 right_;
  // euler Angles
  float yaw_;
  float pitch_;
  float roll_;
  // camera options
  float move_speed_;
  float mouse_sensitivity_;

  CCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), float fov = 45.0f);
  ~CCamera();

  void Init();
  glm::mat4 GetViewMatrix() const;
  glm::mat4 GetProjectionMatrix() const;
  float GetFOV() const;

//  void ProcessKeyboard(CameraMovement direction, float deltaTime);
//  void ProcessMouseMovement(float xoffset, float yoffset,
//                            GLboolean constrainPitch = true);
//  void ProcessMouseScroll(float yoffset);

private:
  // calculates the front vector from the Camera's (updated) Euler Angles
  void UpdateCameraVectors();

  void ProcessCursorPosCallback(double pos_x, double pos_y);

  glm::vec3 world_up_ = glm::vec3(0.0f, 1.0f, 0.0f);
  float fov_;
  float near_;
  float far_;
  bool is_ortho_;
  bool is_cursor_disabled_;
};

} // namespace GEngine
