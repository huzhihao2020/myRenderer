#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <string>

namespace GEngine {
class CEditorUI {
public:
  CEditorUI();
  ~CEditorUI();
  
  void Init();
  void Tick();

  ImGuiIO *GetMainIO() const { return io_; }
  int test_button_status_ = 0;

  const float* GetLightInputs() const { return light_color_;};
  const float* GetColorPickerSphere() const { return sphere_color_;};
  const float* GetColorPickerAreaLight() const { return area_light_color_;};
  const int GetAnimaiton() const { return animation_; }

  float distance_ = 9000.0f;
  float distance_factor_ = 1.0f;
  float view_angle_[2] = {1.39f, 0.0f};
  float sun_angle_[2] = {1.2, 0.7};
  float exposure_ = 10.0f;

  // for precomputed atmosphere scattering
  int texture_level_ = 0; 
  int display_content_ = 0;

private:
  float area_light_color_[4] = {0.7f, 0.7f, 0.7f, 1.0f}; 
  float sphere_color_[4] = {0.7f, 0.05f, 0.05f, 1.0f};  
  float light_color_[4] = {0.7f, 0.7f, 0.7f, 0.3f}; 
  float vec4f_[4] = {0.1f, 0.2f, 0.3f, 0.4f};       // not used yet
  bool show_window_ = true;
  int animation_ = 0;
  ImGuiIO *io_ = nullptr;
  ImGuiStyle *style_;

};
} // namespace GEngine
