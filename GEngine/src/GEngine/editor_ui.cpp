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

  if (ImGui::CollapsingHeader("Animation")) {
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
  }

  if (ImGui::CollapsingHeader("Precomputed Scattering")) {
    ImGui::SliderFloat("height(Km)", &distance_, 7000.0f, 15000.0f);
    ImGui::SliderFloat("height factor", &distance_factor_, 1.0f, 1000.0f);
    ImGui::DragFloat2("view angle", view_angle_, 0.005f, 0.0f, 3.14f);
    if (view_angle_[0] > 1.57)
      view_angle_[0] = 1.57;
    ImGui::DragFloat2("sun angle", sun_angle_, 0.005f, -3.14f, 3.14f);
    ImGui::SliderFloat("exposure", &exposure_, 1.0f, 200.0f);

    ImGui::Text("DisplayContent");
    static int e = 0;
    if (ImGui::SameLine(); ImGui::RadioButton("Scene", &e, 0)) {
      display_content_ = 0;
    }
    if (ImGui::SameLine(); ImGui::RadioButton("T_2D", &e, 1)) {
      display_content_ = 1;
    }
    if (ImGui::SameLine(); ImGui::RadioButton("S_3D", &e, 2)) {
      display_content_ = 2;
    }
    if (ImGui::SameLine(); ImGui::RadioButton("Mie_3D", &e, 3)) {
      display_content_ = 3;
    }
    if (ImGui::SameLine(); ImGui::RadioButton("I_2D", &e, 4)) {
      display_content_ = 4;
    }

    auto SetView = [&](float distance, float view_angle_x, float view_angle_y,
                       float sun_angle_x, float sun_angle_y, float exposure) {
      distance_factor_ = 1.0f;
      distance_ = distance;
      if (distance > 15000.0f) {
        distance_ = 15000.0f;
        distance_factor_ = distance / 15000.0f;
      }
      view_angle_[0] = view_angle_x;
      view_angle_[1] = view_angle_y;
      sun_angle_[0] = sun_angle_x;
      sun_angle_[1] = sun_angle_x;
      exposure_ = exposure;
    };

    if (ImGui::Button("Preset1")) {
      SetView(9000.0, 1.47, 0.0, 1.3, 3.0, 10.0);
    }
    if (ImGui::SameLine(); ImGui::Button("Preset2")) {
      SetView(9000.0, 1.47, 0.0, 1.564, -3.0, 10.0);
    }
    if (ImGui::SameLine(); ImGui::Button("Preset3")) {
      SetView(7000.0, 1.57, 0.0, 1.54, -2.96, 10.0);
    }
    if (ImGui::SameLine(); ImGui::Button("Preset4")) {
      SetView(2.7e6, 0.81, 0.0, 1.57, 2.0, 10.0);
    }
    if (ImGui::SameLine(); ImGui::Button("Preset5")) {
      SetView(1.2e7, 0.0, 0.0, 0.93, -2.0, 10.0);
    }

    ImGui::SliderInt("3D texture level", &texture_level_, 0, 31);
  }
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
