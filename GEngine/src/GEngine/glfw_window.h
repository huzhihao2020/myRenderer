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

  void SetViewport();
  
private:

  GLFWwindow *window_ = nullptr;
};
} // namespace GEngine
