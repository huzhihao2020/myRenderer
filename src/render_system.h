#pragma once
#include <memory>
#include <vector>
#include "singleton.h"
#include "camera.h"
#include "object.h"
#include "glfw_window.h"

namespace GEngine {
  // be sure to call CApp method with CSingleton<RenderSystem>()->func();
class CRenderSystem {
public:
  CRenderSystem();
  ~CRenderSystem();

  void Init();
  std::shared_ptr<CGLFWWindow> GetOrCreateWindow();
  std::shared_ptr<CCamera> GetOrCreateMainCamera();
  // void DrawCube();

private:
  std::shared_ptr<CGLFWWindow> window_;
  std::shared_ptr<CCamera> main_camera_; // main camera
  std::vector<std::shared_ptr<CObject>> render_objects_;

};
} // namespace GEngine