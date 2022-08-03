#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <vector>

#include "common.h"

namespace GEngine {

class CCamera {
public:

  CCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), float fov = 45.0f);
  ~CCamera();

  void Init();
  void Tick();

  glm::mat4 GetViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, world_up_);
  };

  glm::mat4 GetProjectionMatrix() const {
    return is_ortho_ ? glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f)
                     : glm::perspective(glm::radians(fov_),
                                        static_cast<float>(GEngine::WINDOW_CONFIG::VIEWPORT_WIDTH) /static_cast<float>(GEngine::WINDOW_CONFIG::VIEWPORT_HEIGHT),
                                        near_, far_);
  };

  float GetFOV() const { return fov_; };

  bool GetCameraStatus() const { return camera_status_; }
  void SetCameraStatus(bool camera_status) { camera_status_ = camera_status; }
  glm::vec3 GetPosition() const { return position_; }
  glm::vec3 GetFront() const { return front_; }
  glm::vec3 GetRight() const { return right_; }
  glm::vec3 GetUp() const { return up_; }
  float GetMoveSpeed() const { return move_speed_; }
  float GetMouseSensitivity() const { return mouse_sensitivity_; }

private:
  // calculates the front vector from the Camera's (updated) Euler Angles
  void UpdateCameraVectors();

  void ProcessCursorPosCallback(double pos_x, double pos_y);
  void ProcessKeyCallback(int key, int scancode, int action, int mode);
//  void ProcessKeyboard(CameraMovement direction, float deltaTime);
//  void ProcessMouseMovement(float xoffset, float yoffset,GLboolean constrainPitch = true);
//  void ProcessMouseScroll(float yoffset);

  glm::vec3 world_up_ = glm::vec3(0.0f, 1.0f, 0.0f);
  // camera parameters
  float fov_;
  float near_;
  float far_;
  bool is_ortho_;
  bool camera_status_;
  // camera sensitivity
  float move_speed_;
  float mouse_sensitivity_;
  // camera vectors
  glm::vec3 position_;
  glm::vec3 front_;
  glm::vec3 up_;
  glm::vec3 right_;
  // euler Angles
  float yaw_;
  float pitch_;
};

} // namespace GEngine
