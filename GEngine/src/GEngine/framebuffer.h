#pragma once
#include "GEngine/texture.h"
#include "GEngine/renderbuffer.h"

namespace GEngine {

class CAttachment {
public:
  enum class EAttachmentType {
    kNone        = GL_NONE,
    kTexture     = GL_TEXTURE,
    kRnderbuffer = GL_RENDERBUFFER,
  };

  CAttachment() : type_(EAttachmentType::kNone) {}
  CAttachment(const std::shared_ptr<CTexture> &texture)
      : type_(EAttachmentType::kTexture), texture_(texture) {}
  CAttachment(const std::shared_ptr<CRenderBuffer> &renderbuffer)
      : type_(EAttachmentType::kRnderbuffer), renderbuffer_(renderbuffer) {}

  operator bool() const { return type_ != EAttachmentType::kNone; }
  bool operator==(const CAttachment &other) const {
    return type_ == other.type_ && texture_ == other.texture_ &&
           renderbuffer_ == other.renderbuffer_;
  }
  bool operator!=(const CAttachment &other) const {
    return !(*this == other);
  }

  EAttachmentType GetType() const { return type_; }

  std::shared_ptr<CTexture> GetTexture() const { return texture_; }
  std::shared_ptr<CRenderBuffer> GetRenderbuffer() const { return renderbuffer_; }

private:
  EAttachmentType type_;

  std::shared_ptr<CTexture> texture_;
  std::shared_ptr<CRenderBuffer> renderbuffer_;
};

class CFrameBuffer {
public:
  CFrameBuffer();
  ~CFrameBuffer();

  unsigned int GetID() const { return id_; }

  CAttachment &GetColorAttachment() { return color_attachment_; }
  void SetColorAttachment(const CAttachment &attachment);
  CAttachment &GetDepthAttachment() { return depth_attachment_; }
  void SetDepthAttachment(const CAttachment &attachment);
  CAttachment &GetStencilAttachment() { return stencil_attachment_; }
  void SetStencilAttachment(const CAttachment &attachment);

private:
  void SetAttachment(GLenum attachment_point, const CAttachment& attachment);
  unsigned int id_;
  CAttachment color_attachment_;
  CAttachment depth_attachment_;
  CAttachment stencil_attachment_;
};

} // namespace GEngine