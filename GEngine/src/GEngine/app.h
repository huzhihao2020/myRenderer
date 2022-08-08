#pragma once
#include "GEngine/glfw_window.h"
#include "GEngine/singleton.h"
#include "GEngine/render_system.h"
#include <memory>

namespace GEngine {
// be sure to call CApp method with CSingleton<CApp>()->func();
class CApp {
public:
  CApp();
  ~CApp();
  CApp(const CApp &) = delete;
  CApp &operator=(const CApp &) = delete;
  CApp(CApp &&) = delete;
  CApp &operator=(CApp &&) = delete;

  GLvoid Init();
  GLvoid RunMainLoop();

  GLdouble GetCurrentTime() const;
  GLdouble GetDeltaTime() const;
  GLuint GetFPS() const;

private:
  void CalculateTime();

  GLFWwindow *window_;

  GLdouble deltaTime_ = 0.0;
  GLdouble currentTime_ = 0.0;
  GLdouble lastFrameTime_ = 0.0;
  GLdouble deltaTimeCounter_ = 0.0;
  GLuint frameCounter_ = 0;
  GLuint currentFPS_ = 0;
};

} // namespace GEngine
