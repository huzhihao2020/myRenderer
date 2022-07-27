#include "input_system.h"

// static members of CInputSystem
std::vector<std::function<void(int, int, int, int)>>  GEngine::CInputSystem::key_callback_actions_;
std::vector<std::function<void(int, int)>>            GEngine::CInputSystem::frame_size_callback_actions_;
std::vector<std::function<void(double, double)>>      GEngine::CInputSystem::cursor_pos_callback_actions_;

GEngine::CInputSystem::CInputSystem()
{
}
GEngine::CInputSystem::~CInputSystem()
{
}

void GEngine::CInputSystem::Init() {
  auto window = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
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

void GEngine::CInputSystem::KeyCallBackFunction(GLFWwindow *window, GLint key,
                                                GLint scancode, GLint action,
                                                GLint mode) {
  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, GL_TRUE);
      break;
    case GLFW_KEY_W:
      std::cout << "W Pressed!\n";
      break;
    case GLFW_KEY_A:
      std::cout << "A Pressed!\n";
      break;
    case GLFW_KEY_S:
      std::cout << "S Pressed!\n";
      break;
    case GLFW_KEY_D:
      std::cout << "D Pressed!\n";
      break;
    default:
      break;
    }
  }
  if (action == GLFW_RELEASE) {
        switch (key) {
    case GLFW_KEY_ESCAPE:
      break;
    case GLFW_KEY_W:
      std::cout << "W Released!\n";
      break;
    case GLFW_KEY_A:
      std::cout << "A Released!\n";
      break;
    case GLFW_KEY_S:
      std::cout << "S Released!\n";
      break;
    case GLFW_KEY_D:
      std::cout << "D Released!\n";
      break;
    default:
      break;
    }
  }
}

void GEngine::CInputSystem::FrameSizeCallBackFunction(GLFWwindow *window,
                                                      int width, int height) {
  std::cout << "FrameSizeCallBackFunction!\n";
  glViewport(static_cast<GLint>(0), static_cast<GLint>(0),
             static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void GEngine::CInputSystem::CursorPosCallBackFunction(GLFWwindow *window, GLdouble pos_x, GLdouble pos_y) {
  std::cout << "CursorPosCallBackFunction Called\n";
  if(cursor_pos_callback_actions_.empty()) {
    return;
  }
  for(auto fn : cursor_pos_callback_actions_) {
    fn(pos_x, pos_y);
  }
}
// void GEngine::CInputSystem::MouseButtonCallBackFunction();
// void GEngine::CInputSystem::ScrollCallBackFunction();

void GEngine::CInputSystem::Clear() {
  cursor_delta_x_ = 0;
  cursor_delta_y_ = 0;
}

void GEngine::CInputSystem::CalculateCursorDeltaAngles() {
  std::array<int, 2> window_size = {GEngine::WINDOW_CONFIG::WINDOW_WIDTH,
                                    GEngine::WINDOW_CONFIG::WINDOW_HEIGHT};

  if (window_size[0] < 1 || window_size[1] < 1) {
    return;
  }

  auto render_camera = CSingleton<CRenderSystem>()->GetOrCreateMainCamera();
  auto fov = render_camera->GetFOV();

  float cursor_delta_x(glm::radians(static_cast<float>(cursor_delta_x_)));
  float cursor_delta_y(glm::radians(static_cast<float>(cursor_delta_y_)));

  cursor_delta_yaw_ = (cursor_delta_x / (float)window_size[0]) * fov;
  cursor_delta_pitch_ = -(cursor_delta_y / (float)window_size[1]) * fov;
}

void GEngine::CInputSystem::Tick() {
  CalculateCursorDeltaAngles();
  Clear();
}
