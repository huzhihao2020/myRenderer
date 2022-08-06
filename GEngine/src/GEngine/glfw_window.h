#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace GEngine {
class CGLFWWindow {
public:
  CGLFWWindow();
  ~CGLFWWindow();

  void Init();
  GLFWwindow *GetGLFWwindow() const;

private:
  void SetViewport();

  GLFWwindow *window_ = nullptr;
};
} // namespace GEngine
