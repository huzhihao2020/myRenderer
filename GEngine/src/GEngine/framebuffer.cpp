#include "GEngine/framebuffer.h"

GEngine::CFrameBuffer::CFrameBuffer() {
  glGenFramebuffers(1, &id_);
}

GEngine::CFrameBuffer::~CFrameBuffer() {
  glDeleteFramebuffers(1, &id_);
}

void GEngine::CFrameBuffer::SetColorAttachment(const CAttachment &attachment) {
  if(color_attachment_ == attachment) {
    return;
  }
  color_attachment_ = attachment;
  SetAttachment(GL_COLOR_ATTACHMENT0, attachment);
} 

void GEngine::CFrameBuffer::SetDepthAttachment(const CAttachment &attachment) {
    if(depth_attachment_ == attachment) {
    return;
  }
  depth_attachment_ = attachment;
  SetAttachment(GL_DEPTH_ATTACHMENT, attachment);
}

void GEngine::CFrameBuffer::SetStencilAttachment(const CAttachment &attachment) {
    if(stencil_attachment_ == attachment) {
    return;
  }
  stencil_attachment_ = attachment;
  SetAttachment(GL_STENCIL_ATTACHMENT, attachment);
}

void GEngine::CFrameBuffer::SetAttachment(GLenum attachment_point,
                                          const CAttachment &attachment) {
  glBindFramebuffer(GL_FRAMEBUFFER, id_);
  switch (attachment.GetType()) {
  case CAttachment::EAttachmentType::kNone:
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment_point, GL_RENDERBUFFER, 0);
    break;
  case CAttachment::EAttachmentType::kTexture:
    if(auto texture = attachment.GetTexture()){
      glBindTexture(static_cast<GLenum>(texture->GetTarget()), texture->id_);
      // fixme: ibl_pass
      glFramebufferTexture(GL_FRAMEBUFFER, attachment_point, texture->id_, 0);
      // glFramebufferTexture(GL_FRAMEBUFFER, attachment_point, static_cast<GLenum>(texture->GetTarget()), texture->id_);
    }
    else {
      glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_point, GL_RENDERBUFFER, 0, 0);
    }
    break;
  case CAttachment::EAttachmentType::kRnderbuffer:
  if(auto renderbuffer = attachment.GetRenderbuffer()) {
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer->GetID());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment_point, GL_RENDERBUFFER, renderbuffer->GetID());
  } else {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment_point, GL_RENDERBUFFER, 0);
  }
    break;
  }
}
