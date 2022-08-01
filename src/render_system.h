#pragma once
#include "camera.h"
#include "glfw_window.h"
#include "model.h"
#include "shader.h"
#include "singleton.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

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
  std::shared_ptr<CModel>& GetOrCreateModelByPath(const std::string& path);

  void RenderCube(CShader& shader);
  // void CreateSphereWithRadius(float r);
  // void CreateRectWithSize(glm::vec2 size);

  unsigned int LoadTexture(const std::string &path);

private:
  unsigned int cube_VAO_ = 0;
  std::shared_ptr<CGLFWWindow> window_;
  std::shared_ptr<CCamera> main_camera_; // main camera
  std::map<std::string, std::shared_ptr<CModel>> model_map_;
};
} // namespace GEngine
