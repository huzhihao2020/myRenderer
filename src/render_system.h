#pragma once
#include <memory>
#include <vector>
#include "singleton.h"
#include "camera.h"
#include "object.h"
#include "glfw_window.h"
#include "shader.h"

namespace GEngine {
  // be sure to call CApp method with CSingleton<RenderSystem>()->func();
class CRenderSystem {
public:
  // todo: struct VertexAttribute {};

  CRenderSystem();
  ~CRenderSystem();

  void Init();
  std::shared_ptr<CGLFWWindow> GetOrCreateWindow();
  std::shared_ptr<CCamera> GetOrCreateMainCamera();

  void RenderCube(CShader& shader);
  // void CreateSphereWithRadius(float r);
  // void CreateRectWithSize(glm::vec2 size);

private:
  unsigned int cube_VAO_ = 0;
  std::shared_ptr<CGLFWWindow> window_;
  std::shared_ptr<CCamera> main_camera_; // main camera
  std::vector<std::shared_ptr<CObject>> render_objects_;

};
} // namespace GEngine