#pragma once
#include "GEngine/common.h"
#include "GEngine/shader.h"
#include "GEngine/texture.h"
#include "GEngine/framebuffer.h"
#include <string>

namespace GEngine {
class CRenderPass {
public:

  enum class ERenderPassType {
    Default,
    Once,
    // todo: implement other renderpass 
    Delay,
    ScreenQuad,
    ShadowMap,
    ZOnly,
    Opaque,
    Transparent,
    UI,
  };

  CRenderPass();
  CRenderPass(const std::string &name, int order);
  CRenderPass(const std::string &name, int order, ERenderPassType type);
  virtual ~CRenderPass();

  // implemented by user
  virtual void Init() = 0;
  virtual void Tick() = 0;

  bool operator<(const CRenderPass& ohter) const;
  bool operator>(const CRenderPass& ohter) const;

  const std::string &GetName() const { return pass_name_; }
  void SetName(const std::string &name) { pass_name_ = name; }

  const int GetOrder() const { return pass_order_; }
  void SetOrder(int order) { pass_order_ = order; }

  ERenderPassType GetType() const { return pass_type_; }
  void SetType (ERenderPassType type) { pass_type_ = type; }

  std::shared_ptr<Shader> shader_;

private:
  std::string pass_name_;
  int pass_order_ = -1;
  ERenderPassType pass_type_ = ERenderPassType::Default;
};

class RenderPassDesc {
  public:
  RenderPassDesc();
  ~RenderPassDesc();

  void Init();

  private:
  unsigned int fbo_;
  std::shared_ptr<CFrameBuffer> framebuffer_;
  std::shared_ptr<CTexture> color_attachment_;
  std::shared_ptr<CTexture> depth_attachment_;
};

} // namespace GEngine