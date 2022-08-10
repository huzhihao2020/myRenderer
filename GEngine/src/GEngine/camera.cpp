#include "camera.h"
#include <iostream>
#include "input_system.h"
#include "app.h"

GEngine::CCamera::CCamera(glm::vec3 position, float fov)
    : front_(glm::vec3(0.0f, 0.0f, -1.0f)),
      move_speed_(5.0f),
      mouse_sensitivity_(0.3f),
      fov_(fov),
      world_up_(glm::vec3(0.0f, 1.0f, 0.0f)),
      yaw_(0.0f),
      pitch_(0.0f),
      position_(position),
      is_ortho_(false),
      near_(0.1f),
      far_(300.0f) {
  // do nothing
}

GEngine::CCamera::~CCamera()
{
}

void GEngine::CCamera::Init() {
  // todo: register scroll
  CSingleton<CInputSystem>()->RegisterCursorPosCallBackFunction(
      std::bind(&CCamera::ProcessCursorPosCallback, this, std::placeholders::_1,
                std::placeholders::_2));
  CSingleton<CInputSystem>()->RegisterKeyCallBackFunction(std::bind(
      &CCamera::ProcessKeyCallback, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  UpdateCameraVectors();
}

void GEngine::CCamera::Tick() {
  float delta_time = CSingleton<CApp>()->GetDeltaTime();
  if (CSingleton<CInputSystem>()->GetKeyStatus(GLFW_KEY_W)) {
    position_ += front_ * delta_time * move_speed_;
  }
  if (CSingleton<CInputSystem>()->GetKeyStatus(GLFW_KEY_S)) {
    position_ -= front_ * delta_time * move_speed_;
  }
  if (CSingleton<CInputSystem>()->GetKeyStatus(GLFW_KEY_D)) {
    position_ += right_ * delta_time * move_speed_;
  }
  if (CSingleton<CInputSystem>()->GetKeyStatus(GLFW_KEY_A)) {
    position_ -= right_ * delta_time * move_speed_;
  }
  if (CSingleton<CInputSystem>()->GetKeyStatus(GLFW_KEY_UP)) {
    move_speed_ = std::min(move_speed_ + 0.5, 20.0);
  }
  if (CSingleton<CInputSystem>()->GetKeyStatus(GLFW_KEY_DOWN)) {
    move_speed_ = std::max(move_speed_ - 0.5, 0.5);
  }
  if (CSingleton<CInputSystem>()->GetKeyStatus(GLFW_KEY_P)) {
    GE_TRACE("Print Something...");
  }
}

void GEngine::CCamera::ProcessCursorPosCallback(double pos_x, double pos_y) {
  if (camera_status_) {
    return;
  }
  auto offset = CSingleton<CInputSystem>()->GetCursorOffset();
  offset[0] *= mouse_sensitivity_;
  offset[1] *= mouse_sensitivity_;
  yaw_ += offset[0];
  pitch_ += offset[1];
  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (pitch_ > 89.0f)
    pitch_ = 89.0f;
  if (pitch_ < -89.0f)
    pitch_ = -89.0f;
  // calculate
  UpdateCameraVectors();
}

void GEngine::CCamera::ProcessKeyCallback(int key, int scancode, int action, int mode) {
  // todo: 
}

// // processes input received from a mouse scroll-wheel event. Only requires input
// // on the vertical wheel-axis
// void GEngine::Camera::ProcessMouseScroll(float yoffset) {
//   Zoom -= (float)yoffset;
//   if (Zoom < 1.0f)
//     Zoom = 1.0f;
//   if (Zoom > 45.0f)
//     Zoom = 45.0f;
// }

// update camera's [front, right, up] vector acccording to [yaw, pitch]
void GEngine::CCamera::UpdateCameraVectors() {
  // calculate the new Front vector
  glm::vec3 front;
  front.x = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front.y = sin(glm::radians(pitch_));
  front.z = -cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front_ = glm::normalize(front);
  // also re-calculate the Right and Up vector
  right_ = glm::normalize(glm::cross(
      front_, world_up_)); // normalize the vectors, because their length gets
                        // closer to 0 the more you look up or down which
                        // results in slower movement.
  up_ = glm::normalize(glm::cross(right_, front_));
}
