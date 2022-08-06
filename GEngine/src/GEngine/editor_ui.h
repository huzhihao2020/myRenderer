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

private:
  ImVec4 clear_color_ = {0.2f, 0.3f, 0.4f, 1.0f}; // not used yet
  float  vec4f_[4]    = {0.1f, 0.2f, 0.3f, 0.4f}; // not used yet
  bool show_window_ = true;
  ImGuiIO *io_ = nullptr;
  ImGuiStyle *style_;
};
} // namespace GEngine
