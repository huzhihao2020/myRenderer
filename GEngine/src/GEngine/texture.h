#pragma once
#include <GLFW/glfw3.h>
#include <vector>

namespace GEngine {

class CSampler {
public:
  enum class EWrapMode : GLenum {
    kClampToEdge  = GL_CLAMP_TO_EDGE,
    kMirrorRepeat = GL_MIRRORED_REPEAT,
    kRepeat       = GL_REPEAT,
  };
  enum class EMagFilter : GLenum {
    kNearest = GL_NEAREST,
    kLinear  = GL_LINEAR,
  };
  enum class EMinFilter : GLenum {
    kNearest = GL_NEAREST,
    kLinear  = GL_LINEAR,
    kNearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
    kNearestMipmapLinear   = GL_NEAREST_MIPMAP_LINEAR,
    kLinearMipmapNearest  = GL_LINEAR_MIPMAP_NEAREST,
    kLinearMipmapLinear   = GL_LINEAR_MIPMAP_LINEAR,
  };

  static CSampler Create();
  static CSampler CreateFromFiltersAndWrapModes(
      EMinFilter min_filter, EMagFilter mag_filter, EWrapMode s_wrap_mode,
      EWrapMode t_wrap_mode, EWrapMode r_wrap_mode = EWrapMode::kClampToEdge);

  CSampler();
  CSampler(EMinFilter min_filter, EMagFilter mag_filter, EWrapMode s_wrap_mode,
           EWrapMode t_wrap_mode,
           EWrapMode r_wrap_mode = EWrapMode::kClampToEdge);
  ~CSampler();

  EMinFilter GetMinFilter() const { return min_filter_; }
  void SetMinFilter(EMinFilter min_filter) { min_filter_ = min_filter; }
  EMagFilter GetMagFilter() const { return mag_filter_; }
  void SetMagFilter(EMagFilter mag_filter) { mag_filter_ = mag_filter; }
  EWrapMode GetSWrapMode() const { return s_wrap_mode_; }
  void SetSWrapMode(EWrapMode s_wrap_mode) { s_wrap_mode_ = s_wrap_mode; }
  EWrapMode GetTWrapMode() const { return t_wrap_mode_; }
  void SetTWrapMode(EWrapMode t_wrap_mode) { t_wrap_mode_ = t_wrap_mode; }
  EWrapMode GetRWrapMode() const { return r_wrap_mode_; }
  void SetRWrapMode(EWrapMode r_wrap_mode) { r_wrap_mode_ = r_wrap_mode; }

private:
  EMagFilter mag_filter_ = EMagFilter::kLinear;
  EMinFilter min_filter_ = EMinFilter::kLinear;
  EWrapMode s_wrap_mode_ = EWrapMode::kClampToEdge;
  EWrapMode t_wrap_mode_ = EWrapMode::kClampToEdge;
  EWrapMode r_wrap_mode_ = EWrapMode::kClampToEdge;
};

// class CTexture : puhlic std::enable_shared_fron_this<CTexture>{
class CTexture {
public:
  enum class ETarget : GLenum {
    kTexture2D      = GL_TEXTURE_2D,
    kTexture3D      = GL_TEXTURE_3D,
    kTextureCubeMap = GL_TEXTURE_CUBE_MAP,
  };

  enum class EPixelFormat : GLenum {
    kRed  = GL_RED,
    kRGB  = GL_RGB,
    kRGBA = GL_RGBA,
  };

  enum class EWrapMode : GLenum {
    kClampToEdge  = GL_CLAMP_TO_EDGE,
    kMirrorRepeat = GL_MIRRORED_REPEAT,
    kRepeat       = GL_REPEAT,
  };
  enum class EMagFilter : GLenum {
    kNearest = GL_NEAREST,
    kLinear  = GL_LINEAR,
  };
  enum class EMinFilter : GLenum {
    kNearest = GL_NEAREST,
    kLinear  = GL_LINEAR,
    kNearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
    kNearestMipmapLinear   = GL_NEAREST_MIPMAP_LINEAR,
    kLinearMipmapNearest  = GL_LINEAR_MIPMAP_NEAREST,
    kLinearMipmapLinear   = GL_LINEAR_MIPMAP_LINEAR,
  };

  CTexture(ETarget target);
  CTexture(std::string& path, ETarget target = ETarget::kTexture2D);
  CTexture(ETarget target, unsigned int id, int height, int width);
  ~CTexture();

  static CTexture CreateTextureFromFile();

  ETarget GetTarget() const { return target_; }

  int GetWidth() const { return width_; }
  void SetWidth(int width) { width_ = width; }
  int GetHeight() const { return height_; }
  void SetHeight(int height) { height_ = height; }

  EMinFilter GetMinFilter() const { return min_filter_; }
  void SetMinFilter(EMinFilter min_filter) { min_filter_ = min_filter; }
  EMagFilter GetMagFilter() const { return mag_filter_; }
  void SetMagFilter(EMagFilter mag_filter) { mag_filter_ = mag_filter; }
  EWrapMode GetSWrapMode() const { return s_wrap_mode_; }
  void SetSWrapMode(EWrapMode s_wrap_mode) { s_wrap_mode_ = s_wrap_mode; }
  EWrapMode GetTWrapMode() const { return t_wrap_mode_; }
  void SetTWrapMode(EWrapMode t_wrap_mode) { t_wrap_mode_ = t_wrap_mode; }
  EWrapMode GetRWrapMode() const { return r_wrap_mode_; }
  void SetRWrapMode(EWrapMode r_wrap_mode) { r_wrap_mode_ = r_wrap_mode; }

  unsigned int id_;
  std::vector<GLvoid*> datas_;
  EPixelFormat internal_format_ = EPixelFormat::kRGB; 
  EPixelFormat external_format_ = EPixelFormat::kRGB;

private:
  ETarget target_;
  bool owner_;

  int width_ = 0;
  int height_ = 0;

  EMagFilter mag_filter_ = EMagFilter::kLinear;
  EMinFilter min_filter_ = EMinFilter::kLinear;
  EWrapMode s_wrap_mode_ = EWrapMode::kClampToEdge;
  EWrapMode t_wrap_mode_ = EWrapMode::kClampToEdge;
  EWrapMode r_wrap_mode_ = EWrapMode::kClampToEdge;

  bool has_mipmap_ = false;
};
} // namespace GEngine
