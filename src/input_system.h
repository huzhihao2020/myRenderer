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

  void RegisterKeyCallBackFunction(std::function<void(int, int, int, int)> key_callback_function);
  void RegisterFrameSizeCallBackFunction(std::function<void(int, int)> frame_size_callback_function);
  void RegisterCursorPosCallBackFunction(std::function<void(double, double)> cursor_pos_callback_function);

  const std::array<double, 2>& GetCursorPos() const;
  const std::array<double, 2>& GetCursorOffset() const;

  const short GetKeyStatus(int key) const;

private:

  static std::array<short, 512> key_status_;  // 0: release, 1: press, 2: repeat
  static std::array<short, 3> cursor_status_; // todo

  static void KeyCallBackFunction(GLFWwindow *window, int key, int scancode, int action, int mode);
  static void FrameSizeCallBackFunction(GLFWwindow *window, int width, int height);
  static void CursorPosCallBackFunction(GLFWwindow *window, double pos_x, double pos_y);
  // static void GEngine::CInputSystem::MouseButtonCallBackFunction(GLFWwindow *window,);
  // static void GEngine::CInputSystem::ScrollCallBackFunction(GLFWwindow *window,);

  static std::vector<std::function<void(int, int, int, int)>> key_callback_actions_;
  static std::vector<std::function<void(int, int)>>           frame_size_callback_actions_;
  static std::vector<std::function<void(double, double)>>     cursor_pos_callback_actions_;

  static std::array<double, 2> current_cursor_;
  static std::array<double, 2> last_cursor_;
  static std::array<double, 2> cursor_offset_;
};
} // namespace GEngine