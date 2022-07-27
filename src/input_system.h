#pragma once
#include "render_system.h"
#include "common.h"
#include <iostream>
#include <array>
#include <functional>
#include <vector>
#include "singleton.h"

namespace GEngine {
class CInputSystem {
public:
    enum class EMovementCommand : unsigned int
    {
        forward  = 1 << 0,                 // W
        backward = 1 << 1,                 // S
        left     = 1 << 2,                 // A
        right    = 1 << 3,                 // D
        jump     = 1 << 4,                 // SPACE
        squat    = 1 << 5,                 // not implemented yet
        sprint   = 1 << 6,                 // LEFT SHIFT
        fire     = 1 << 7,                 // not implemented yet
        invalid  = (unsigned int)(1 << 31) // lost focus
    };

  CInputSystem();
  ~CInputSystem();

  void Init();
  void Clear();
  void Tick();

  void RegisterKeyCallBackFunction(std::function<void(int, int, int, int)> key_callback_function);
  void RegisterFrameSizeCallBackFunction(std::function<void(int, int)> frame_size_callback_function);
  void RegisterCursorPosCallBackFunction(std::function<void(double, double)> cursor_pos_callback_function);

  int cursor_delta_x_{0};
  int cursor_delta_y_{0};
  float cursor_delta_yaw_{0.0};
  float cursor_delta_pitch_{0.0};

private:
  void CalculateCursorDeltaAngles();

  static void KeyCallBackFunction(GLFWwindow *window, GLint key, GLint scancode, GLint action, GLint mode);
  static void FrameSizeCallBackFunction(GLFWwindow *window, int width, int height);
  static void CursorPosCallBackFunction(GLFWwindow *window, GLdouble pos_x, GLdouble pos_y);
  // static void GEngine::CInputSystem::MouseButtonCallBackFunction(GLFWwindow *window,);
  // static void GEngine::CInputSystem::ScrollCallBackFunction(GLFWwindow *window,);

  static std::vector<std::function<void(int, int, int, int)>> key_callback_actions_;
  static std::vector<std::function<void(int, int)>>           frame_size_callback_actions_;
  static std::vector<std::function<void(double, double)>>     cursor_pos_callback_actions_;

  int last_cursor_x_{0};
  int last_cursor_y_{0};
};
} // namespace GEngine