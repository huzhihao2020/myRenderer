#include "camera.h"
#include "common.h"
#include <iostream>
GEngine::CCamera::CCamera(glm::vec3 position, float fov)
    : front_(glm::vec3(0.0f, 0.0f, -1.0f)),
      move_speed_(5.0f),
      mouse_sensitivity_(0.1f),
      fov_(fov),
      world_up_(glm::vec3(0.0f, 1.0f, 0.0f)),
      yaw_(-90.f),
      pitch_(0.0f),
      position_(position),
      is_ortho_(false),
      near_(0.1f),
      far_(100.0f) {
  Init();
  UpdateCameraVectors();
}

GEngine::CCamera::~CCamera()
{
}

void GEngine::CCamera::Init() {
  std::cout << "Camera Init!\n"; 
  // register callback
}

glm::mat4 GEngine::CCamera::GetViewMatrix() const {
  return glm::lookAt(position_, position_ + front_, world_up_);
}
glm::mat4 GEngine::CCamera::GetProjectionMatrix() const {
  return is_ortho_ ? glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f)
                   : glm::perspective(glm::radians(fov_),
                         static_cast<float>(GEngine::WINDOW_CONFIG::VIEWPORT_WIDTH) / static_cast<float>(GEngine::WINDOW_CONFIG::VIEWPORT_HEIGHT),
                         near_, far_);
}

// // processes input received from any keyboard-like input system. Accepts input
// // parameter in the form of camera defined ENUM (to abstract it from windowing
// // systems)
// void GEngine::Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
//   float velocity = MovementSpeed * deltaTime;
//   if (direction == FORWARD)
//     Position += Front * velocity;
//   if (direction == BACKWARD)
//     Position -= Front * velocity;
//   if (direction == LEFT)
//     Position -= Right * velocity;
//   if (direction == RIGHT)
//     Position += Right * velocity;
// }

// // processes input received from a mouse input system. Expects the offset value
// // in both the x and y direction.
// void GEngine::Camera::ProcessMouseMovement(float xoffset, float yoffset,
//                           GLboolean constrainPitch = true) {
//   xoffset *= MouseSensitivity;
//   yoffset *= MouseSensitivity;

//   Yaw += xoffset;
//   Pitch += yoffset;

//   // make sure that when pitch is out of bounds, screen doesn't get flipped
//   if (constrainPitch) {
//     if (Pitch > 89.0f)
//       Pitch = 89.0f;
//     if (Pitch < -89.0f)
//       Pitch = -89.0f;
//   }

//   // update Front, Right and Up Vectors using the updated Euler angles
//   updateCameraVectors();
// }

// // processes input received from a mouse scroll-wheel event. Only requires input
// // on the vertical wheel-axis
// void GEngine::Camera::ProcessMouseScroll(float yoffset) {
//   Zoom -= (float)yoffset;
//   if (Zoom < 1.0f)
//     Zoom = 1.0f;
//   if (Zoom > 45.0f)
//     Zoom = 45.0f;
// }

// calculates the front vector from the Camera's (updated) Euler Angles
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
