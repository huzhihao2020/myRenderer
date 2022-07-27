#pragma once
#include <memory>
#include "glfw_window.h"

namespace GEngine {

class CApp {
public:
  // singleton
  ~CApp();
  CApp(const CApp &) = delete;
  CApp &operator=(const CApp &) = delete;
  CApp(CApp &&) = delete;
  CApp &operator=(CApp &&) = delete;

  static auto &Instance() {
    static CApp app;
    return app;
  }

GLvoid Init();
GLvoid RunMainLoop();

GLdouble GetCurrentTime() const;
GLdouble GetDeltaTime() const;
GLuint GetFPS() const;

private:
  CApp(); // Disallow instantiation outside of the class.
  void CalculateTime();

  GLFWwindow *window_;
  GLdouble  deltaTime_ = 0.0;
  GLdouble  currentTime_ = 0.0;
  GLdouble  lastFrameTime_ = 0.0;
  GLdouble  deltaTimeCounter_ = 0.0;
  GLuint    frameCounter_ = 0;
  GLuint    currentFPS_ = 0;
};

} // namespace GEngine
