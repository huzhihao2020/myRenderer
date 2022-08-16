#include "GEngine/renderbuffer.h"
#include "log.h"

GEngine::CRenderBuffer::CRenderBuffer()
    : owner_(true), width_(0), height_(0), internal_format_(EPixelFormat::kRGBA8) {
  glGenRenderbuffers(1, &id_);
  if (id_ <= 0) {
    GE_ERROR("Failed to create renderbuffer");
  }
}

GEngine::CRenderBuffer::CRenderBuffer(unsigned int id) : id_(id), owner_(false) {
  if(id_ <= 0) {
    GE_ERROR("Failed to create renderbuffer");
  }
}

GEngine::CRenderBuffer::~CRenderBuffer() {
  if(owner_) {
    glDeleteRenderbuffers(1, &id_);
  }
}

void GEngine::CRenderBuffer::InitialzeStorage(EPixelFormat internal_format, unsigned int width, unsigned int height) {
  glBindRenderbuffer(GL_RENDERBUFFER, id_);
  internal_format_ = internal_format;
  width_ = width;
  height_ = height;
  glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(internal_format_), width_, height_);
}