#pragma once
#include <glad/glad.h>

namespace GEngine {

class CRenderBuffer {
public:
  enum class EPixelFormat {
    kRGBA8 = GL_RGBA8,
    kDepthComponent24 = GL_DEPTH_COMPONENT24,
    kDepth24Stencil8 = GL_DEPTH24_STENCIL8,
    kStencil8 = GL_STENCIL_INDEX8,
  };

  CRenderBuffer();
  CRenderBuffer(unsigned int id);
  ~CRenderBuffer();

  void InitialzeStorage(EPixelFormat internal_format, unsigned int width, unsigned int height);

  unsigned int GetID() const { return id_; }
  EPixelFormat GetInternalFormat() const { return internal_format_; }
  unsigned int GetWidth() const { return width_; }
  unsigned int GetHeight() const { return height_; }

private:
  unsigned int id_;
  bool owner_;

  EPixelFormat internal_format_;
  unsigned int width_;
  unsigned int height_;
};

} // namespace GEngine