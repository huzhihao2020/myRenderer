#pragma once
#include "GEngine/camera.h"
#include "GEngine/editor_ui.h"
#include "GEngine/glfw_window.h"
#include "GEngine/model.h"
#include "GEngine/render_pass.h"
#include "GEngine/shader.h"
#include "GEngine/singleton.h"
#include <initializer_list>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

namespace GEngine {
  // be sure to call CApp method with CSingleton<RenderSystem>()->func();
class CRenderSystem {
public:
  enum class ERenderPipelineType : uint8_t {
    kForward = 0,
    kDeferred,
    kPipelineTypeNum
  };

  CRenderSystem();
  ~CRenderSystem();

  void Init();
  std::shared_ptr<CGLFWWindow>  GetOrCreateWindow();
  std::shared_ptr<CCamera>      GetOrCreateMainCamera();
  std::shared_ptr<CEditorUI>    GetOrCreateMainUI();
  // std::shared_ptr<CModel>&      GetOrCreateModelByPath(const std::string& path);
  std::vector<std::shared_ptr<GEngine::CRenderPass>>& GetRenderPass() { return render_passes_; }
  void SetRenderPipelineType(ERenderPipelineType type);

  void RenderCube();
  void RenderCube(std::shared_ptr<CShader> shader);
  void RenderSphere(std::shared_ptr<CShader> shader);
  
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
  unsigned int sphere_VAO_ = 0;
  unsigned int sphere_index_count_ = 0;
  std::shared_ptr<CGLFWWindow>  window_;
  std::shared_ptr<CCamera>      main_camera_; // main camera
  std::shared_ptr<CEditorUI>    main_UI_;     // main UI

  std::vector<std::shared_ptr<GEngine::CRenderPass>>  render_passes_;
  ERenderPipelineType render_pipeline_type_ = ERenderPipelineType::kForward;

};
} // namespace GEngine
