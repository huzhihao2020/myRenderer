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
#include "render_pass.h"
#include <initializer_list>
#include <numeric>

namespace GEngine {
  // be sure to call CApp method with CSingleton<RenderSystem>()->func();
class CRenderSystem {
public:
  // todo: struct VertexAttribute {};

  CRenderSystem();
  ~CRenderSystem();

  void Init();
  std::shared_ptr<CGLFWWindow>  GetOrCreateWindow();
  std::shared_ptr<CCamera>      GetOrCreateMainCamera();
  std::shared_ptr<CModel>&      GetOrCreateModelByPath(const std::string& path);
  std::vector<std::shared_ptr<GEngine::CRenderPass>>& GetRenderPass() { return render_passes_; }

  void RenderCube();
  void RenderCube(std::shared_ptr<CShader> shader);
  int GetOrCreateCubeVAO();
  int CreateVAO(const GLvoid *vertex_data, int data_size,
                std::initializer_list<int> attribute_layout,
                const int indices[] = nullptr, int indices_size = 0,
                int *VBO = nullptr);
  // void CreateSphereWithRadius(float r);
  // void CreateRectWithSize(glm::vec2 size);

  unsigned int LoadTexture(const std::string &path);
  void RegisterRenderPass(const std::shared_ptr<GEngine::CRenderPass>& render_pass);

private:
  unsigned int cube_VAO_ = 0;
  std::shared_ptr<CGLFWWindow> window_;
  std::shared_ptr<CCamera> main_camera_; // main camera

  std::vector<std::shared_ptr<GEngine::CRenderPass>>  render_passes_;
  std::map<std::string, std::shared_ptr<CModel>>      model_map_;
};
} // namespace GEngine
