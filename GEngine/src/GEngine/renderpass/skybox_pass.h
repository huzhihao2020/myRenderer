#pragma once
#include "GEngine/render_pass.h"
#include "GEngine/texture.h"
#include <string>

namespace GEngine {
class CSkyboxPass : public CRenderPass {
public:
  CSkyboxPass(const std::string& name, int order);
  virtual ~CSkyboxPass();

  void LoadCubemapFromFiles(const std::vector<std::string>& paths, std::shared_ptr<GEngine::CTexture> texture);

  virtual void Init() override;
  virtual void Tick() override;
private:
  std::shared_ptr<GEngine::CTexture> skybox_texture_;
};
} // namespace GEngine
