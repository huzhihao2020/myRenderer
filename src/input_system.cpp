#include "input_system.h"
#include "log.h"

// static members of CInputSystem
std::vector<std::function<void(int, int, int, int)>>  GEngine::CInputSystem::key_callback_actions_;
std::vector<std::function<void(int, int)>>            GEngine::CInputSystem::frame_size_callback_actions_;
std::vector<std::function<void(double, double)>>      GEngine::CInputSystem::cursor_pos_callback_actions_;
unsigned short     GEngine::CInputSystem::cursor_status_   = 0;  // 0: disabled, 1:normal
std::array<short, 512>  GEngine::CInputSystem::key_status_      = {0};  // 0: release, 1: press, 2: repeat
std::array<double, 2>   GEngine::CInputSystem::current_cursor_  = { 0.0 };
std::array<double, 2>   GEngine::CInputSystem::last_cursor_     = { 0.0 };
std::array<double, 2>   GEngine::CInputSystem::cursor_offset_   = { 0.0 };

GEngine::CInputSystem::CInputSystem()
{
}

GEngine::CInputSystem::~CInputSystem()
{
}

void GEngine::CInputSystem::Init() {
  auto window = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
  // note that these callbacks are not called each frame 
  glfwSetKeyCallback(window, KeyCallBackFunction);
  glfwSetFramebufferSizeCallback(window, FrameSizeCallBackFunction);
  glfwSetCursorPosCallback(window, CursorPosCallBackFunction);
  // glfwSetMouseButtonCallback(window, MouseButtonCallBackFunction);
  // glfwSetScrollCallback(window, ScrollCallBackFunction);
}

void GEngine::CInputSystem::RegisterKeyCallBackFunction(std::function<void(int, int, int, int)> key_callback_function) {
  key_callback_actions_.push_back(key_callback_function);
}
void GEngine::CInputSystem::RegisterFrameSizeCallBackFunction(std::function<void(int, int)> frame_size_callback_function) {
  frame_size_callback_actions_.push_back(frame_size_callback_function);
}
void GEngine::CInputSystem::RegisterCursorPosCallBackFunction(std::function<void(double, double)> cursor_pos_callback_function) {
  cursor_pos_callback_actions_.push_back(cursor_pos_callback_function);
}

const std::array<double, 2>& GEngine::CInputSystem::GetCursorPos() const {
  return current_cursor_;
}

const std::array<double, 2>& GEngine::CInputSystem::GetCursorOffset() const {
  return cursor_offset_;
}

const short GEngine::CInputSystem::GetKeyStatus(int key) const {
  auto window = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
  return glfwGetKey(window, key);
}

void GEngine::CInputSystem::KeyCallBackFunction(GLFWwindow *window, int key,
                                                int scancode, int action,
                                                int mode) {
  // press [esc] to close the window
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  key_status_[key] = action;

  // press [Q] to lock/unlock the cursor
  if(key_status_[GLFW_KEY_Q] == GLFW_PRESS) {
    cursor_status_ = !cursor_status_;
    CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->SetCameraStatus(cursor_status_);
    glfwSetInputMode(window, GLFW_CURSOR, cursor_status_==0 ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
  }

  if (key_callback_actions_.empty()) {
    return;
  }

  for (auto fn : key_callback_actions_) {
    fn(key, scancode, action, mode);
  }
}

void GEngine::CInputSystem::FrameSizeCallBackFunction(GLFWwindow *window,
                                                      int width, int height) {
  glViewport(static_cast<GLint>(0), static_cast<GLint>(0),
             static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void GEngine::CInputSystem::CursorPosCallBackFunction(GLFWwindow *window, double pos_x, double pos_y) {
  current_cursor_[0] = pos_x;
  current_cursor_[1] = pos_y;

  cursor_offset_[0] = current_cursor_[0] - last_cursor_[0];
  cursor_offset_[1] = last_cursor_[1] - current_cursor_[1];

  last_cursor_ = current_cursor_;

  if(cursor_pos_callback_actions_.empty()) {
    return;
  }
  for(auto fn : cursor_pos_callback_actions_) {
    fn(pos_x, pos_y);
  }
}
// void GEngine::CInputSystem::MouseButtonCallBackFunction();
// void GEngine::CInputSystem::ScrollCallBackFunction();
