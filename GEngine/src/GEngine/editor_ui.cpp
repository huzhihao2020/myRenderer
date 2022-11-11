#include "editor_ui.h"
#include "app.h"
#include "glfw_window.h"
#include "imgui.h"
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
  ImGui_ImplOpenGL3_Init("#version 410");
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
  ImGui::SameLine(); ImGui::Text("Mouse speed: %.2f", camera->GetMoveSpeed());
  if(ImGui::Button("Debug Mode")) { 
    GE_INFO("Button Clicked."); 
    test_button_status_ ^= 1;
  }
  // ImGui::DragFloat3("Test", vec4f_, 0.01f, 0.0f, 1.0f);
  // ImGui::SliderFloat3("light color", light_color_, 0.0f, 1.0f);
  ImGui::ColorEdit3("Sphere Color", sphere_color_); // color picker
  ImGui::DragFloat4("light (rgbI)", light_color_, 0.001f, 0.0f, 1.0f);
  // animation
  if(ImGui::Button("Animaton0")) {
    animation_ = 0;
  }
  if(ImGui::SameLine();ImGui::Button("Animaton1")) {
    animation_ = 1;
  }
  if(ImGui::SameLine();ImGui::Button("Animaton2")) {
    animation_ = 2;
  }
  if(ImGui::SameLine();ImGui::Button("Animaton3")) {
    animation_ = 3;
  }
  ImGui::SliderFloat("view distance", &distance_, 7000.0f, 1000000.0f);
  ImGui::DragFloat2("view angle", view_angle_, 0.005f, 0.0f, 1.57f);
  ImGui::DragFloat2("sun angle", sun_angle_, 0.005f, -3.0f, 3.0f);

  ImGui::Text("DisplayContent");
  if(ImGui::SameLine();ImGui::Button("Scene")) {
    display_content_ = 0;
  }
  if(ImGui::SameLine();ImGui::Button("T_2D")) {
    display_content_ = 1;
  }
  if(ImGui::SameLine();ImGui::Button("S_3D")) {
    display_content_ = 2;
  }
  if(ImGui::SameLine();ImGui::Button("Mie_3D")) {
    display_content_ = 3;
  }
  if(ImGui::SameLine();ImGui::Button("I_2D")) {
    display_content_ = 4;
  }

  ImGui::SliderInt("3D texture level", &texture_level_, 0, 31);

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
