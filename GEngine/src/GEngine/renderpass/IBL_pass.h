#pragma once
#include "GEngine/render_pass.h"
#include <string>

namespace GEngine {
class CIBLPass : public CRenderPass {
public:
  CIBLPass(const std::string &name, int order);
  virtual ~CIBLPass();

  void GenerateBRDFLUT();
  void GenerateIrradianceMap(std::shared_ptr<GEngine::CTexture> texture);
  void GeneratePrefilteredMap(std::shared_ptr<GEngine::CTexture> texture, int max_mip_levels) ;

  virtual void Init() override;
  virtual void Tick() override;

private:
  std::shared_ptr<GEngine::CTexture> irradiance_texture_;
  std::shared_ptr<GEngine::CTexture> prefiltered_texture_;
  std::shared_ptr<GEngine::CTexture> specular_brdf_lut_;
  std::shared_ptr<GEngine::CFrameBuffer> framebuffer_;
};
} // namespace GEngine
