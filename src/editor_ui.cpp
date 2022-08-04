#include "editor_ui.h"
#include "app.h"
#include "glfw_window.h"
#include "log.h"
#include "render_system.h"
#include <glm/glm.hpp>

GEngine::CEditorUI::CEditorUI()
{
}

GEngine::CEditorUI::~CEditorUI() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void GEngine::CEditorUI::Init() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io_ = &ImGui::GetIO(); (void)(*io_);
  io_->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io_->ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;  // Enable Mouse Controls
  io_->ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
  io_->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  style_ = &ImGui::GetStyle();
  if (io_->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style_->WindowRounding = 0.0f;
    style_->Colors[ImGuiCol_WindowBg].w = 1.0f;
    style_->Alpha = 0.8f;
  }
  // Setup Platform/Renderer backends
  auto window = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void GEngine::CEditorUI::Tick() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  int w_width, w_height;
  auto window = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
  glfwGetWindowSize(window, &w_width, &w_height);
  // imgui begin
  // ImGuiWindowFlags window_flag = 0;
  // window_flag |= ImGuiWindowFlags_MenuBar;
  // ImGui::Begin("Menu", &show_window_, window_flag);
  ImGui::Begin("Menu");
  ImGui::TextWrapped("Window Size: %d, %d. ", w_width, w_height);ImGui::SameLine();ImGui::Text("[ESC]: Close Window");
  auto camera = CSingleton<CRenderSystem>()->GetOrCreateMainCamera();
  auto camera_position = camera->GetPosition();
  auto camera_front = camera->GetFront();
  ImGui::Text("CameraPos: (%.3f, %.3f, %.3f)", camera_position.x, camera_position.y, camera_position.z);
  ImGui::Text("CameraFront: (%.3f, %.3f, %.3f)", camera_front.x, camera_front.y, camera_front.z);
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::Text("[Q]: Enable/Disable Cursor");
  ImGui::Text("[Up/Down]: Change Move Speed of Camera"); 
  ImGui::Text("MousePos: %d, %d", static_cast<int>(io_->MousePos.x), static_cast<int>(io_->MousePos.y));
  if(ImGui::Button("Test Button")) { GE_INFO("Test Button Clicked."); }
  ImGui::InputFloat3("input float3", vec4f_);
  ImGui::DragFloat3("drag float3", vec4f_, 0.01f, 0.0f, 1.0f);
  ImGui::SliderFloat3("slider float3", vec4f_, 0.0f, 1.0f);
  ImGui::ColorEdit3("clear color", (float*)&clear_color_); // color picker
  ImGui::End();
  // imgui end
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (io_->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}
