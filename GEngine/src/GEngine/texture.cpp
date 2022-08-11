#include "GEngine/texture.h"
#include "GEngine/log.h"
#include <stb/stb_image.h>

GEngine::CSampler::CSampler() {}

GEngine::CSampler::CSampler(EMinFilter min_filter, EMagFilter mag_filter,
                            EWrapMode s_wrap_mode, EWrapMode t_wrap_mode,
                            EWrapMode r_wrap_mode)
    : min_filter_(min_filter), mag_filter_(mag_filter),
      s_wrap_mode_(s_wrap_mode), t_wrap_mode_(t_wrap_mode),
      r_wrap_mode_(r_wrap_mode) {}

GEngine::CSampler::~CSampler() {}

GEngine::CSampler GEngine::CSampler::Create() { return CSampler(); }

GEngine::CSampler GEngine::CSampler::CreateFromFiltersAndWrapModes(
    EMinFilter min_filter, EMagFilter mag_filter,
    EWrapMode s_wrap_mode, EWrapMode t_wrap_mode, EWrapMode r_wrap_mode) {
  return CSampler(min_filter, mag_filter, s_wrap_mode, t_wrap_mode, r_wrap_mode);
}

GEngine::CTexture::CTexture(ETarget target) : target_(target), owner_(true) {
  glGenTextures(1, &id_);
}

GEngine::CTexture::CTexture(std::string &path, ETarget target, bool need_flip) {
  target_ = target;
  owner_ = true;
  stbi_set_flip_vertically_on_load(need_flip);
  switch (target) {
  case ETarget::kTexture2D: {
    glGenTextures(1, &id_);
    int components_number = 0;
    unsigned char *data = stbi_load(path.c_str(), &width_, &height_, &components_number, 0);
    if (data) {
      if (components_number == 1)
        internal_format_ = external_format_ = EPixelFormat::kRed;
        
      else if (components_number == 3)
        internal_format_ = external_format_ = EPixelFormat::kRGB;
      else if (components_number == 4)
        internal_format_  = external_format_ = EPixelFormat::kRGBA;

      glBindTexture(GL_TEXTURE_2D, id_);
      glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(internal_format_),
                   width_, height_, 0, static_cast<GLenum>(external_format_),
                   GL_UNSIGNED_BYTE, data);
      has_mipmap_ = true;
      glGenerateMipmap(GL_TEXTURE_2D);
      SetMinFilter(EMinFilter::kLinearMipmapLinear);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(s_wrap_mode_));
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(s_wrap_mode_));
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(min_filter_));
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(mag_filter_));
      stbi_image_free(data);
    } else {
      GE_ERROR("Texture failed to load at path: {0}", path);
      stbi_image_free(data);
    }
  } break;
  default:
    GE_ERROR("Cannot create texture (target not supported) from path: {0}", path);
    break;
  }
}

GEngine::CTexture::CTexture(ETarget target, unsigned int id, int height, int width)
    : id_(id), target_(target), width_(width), height_(height), owner_(false) {}

GEngine::CTexture::~CTexture() {
  if(owner_) {
    glDeleteTextures(1, &id_);
  }
}
