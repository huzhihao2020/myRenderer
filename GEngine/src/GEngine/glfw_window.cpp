#include "glfw_window.h"
#include "common.h"
#include <iostream>
#include "log.h"

GEngine::CGLFWWindow::CGLFWWindow() : window_(nullptr) {}

GEngine::CGLFWWindow::~CGLFWWindow() {}

void GEngine::CGLFWWindow::Init() {
  // glfw: initialize and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // multi-sample?
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  // glfw window setup
  window_ = glfwCreateWindow(WINDOW_CONFIG::WINDOW_WIDTH,
                             WINDOW_CONFIG::WINDOW_HEIGHT,
                             WINDOW_CONFIG::WINDOW_TITLE.c_str(), nullptr, nullptr);
  if (!window_) {
    GE_CORE_ERROR("Failed to create GLFW window");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1); // Enable vsync
  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetInputMode(window_, GLFW_STICKY_KEYS, GLFW_TRUE);
  glfwSetCursorPos(window_, WINDOW_CONFIG::WINDOW_WIDTH/2, WINDOW_CONFIG::WINDOW_HEIGHT/2); // todo: fix me
  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    GE_CORE_ERROR("Failed to initialize GLAD");
    return;
  }
  SetViewport();
  GE_CORE_INFO("GLFW window Init!");
  return;
}

void GEngine::CGLFWWindow::SetViewport() {
  int factor = WINDOW_CONFIG::IS_MACOS_WINDOW ? 2 : 1;
  glViewport(static_cast<GLint>(WINDOW_CONFIG::VIEWPORT_LOWERLEFT_X),
             static_cast<GLint>(WINDOW_CONFIG::VIEWPORT_LOWERLEFT_Y),
             static_cast<GLsizei>(WINDOW_CONFIG::VIEWPORT_WIDTH * factor),
             static_cast<GLsizei>(WINDOW_CONFIG::VIEWPORT_HEIGHT * factor));
}

GLFWwindow* GEngine::CGLFWWindow::GetGLFWwindow() const {
  return window_;
}
