#include "glfw_window.h"
#include "common.h"
#include <iostream>

GEngine::CGLFWWindow::CGLFWWindow() : window_(nullptr) {}

GEngine::CGLFWWindow::~CGLFWWindow() {}

void GEngine::CGLFWWindow::Init() {
  // glfw: initialize and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window_);
  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  glfwSetInputMode(window_, GLFW_STICKY_KEYS, GLFW_TRUE);
  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return;
  }
  SetViewport();
  // stbi_set_flip_vertically_on_load(true);
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
