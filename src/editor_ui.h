#pragma once
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
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
  ImVec4 clear_color_;
  bool show_window_ = true;
  ImGuiIO *io_ = nullptr;
  ImGuiStyle *style_;
};
} // namespace GEngine
