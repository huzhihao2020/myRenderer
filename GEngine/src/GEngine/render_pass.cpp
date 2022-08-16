#include "GEngine/render_pass.h"
#include "GEngine/texture.h"

GEngine::CRenderPass::CRenderPass()
{
}

GEngine::CRenderPass::CRenderPass(const std::string &name, int order)
    : pass_name_(name), pass_order_(order) 
{
}

GEngine::CRenderPass::CRenderPass(const std::string &name, int order,
                                  ERenderPassType type)
    : pass_name_(name), pass_order_(order), pass_type_(type)
{
}

GEngine::CRenderPass::~CRenderPass()
{
}

bool GEngine::CRenderPass::operator<(const CRenderPass &ohter) const {
  return pass_order_ < ohter.GetOrder();
}

bool GEngine::CRenderPass::operator>(const CRenderPass &ohter) const {
  return pass_order_ > ohter.GetOrder();
}

GEngine::RenderPassDesc::RenderPassDesc() {}

GEngine::RenderPassDesc::~RenderPassDesc() {}

void GEngine::RenderPassDesc::Init() { 
  glGenFramebuffers(1, &fbo_); 
}
