#include "GEngine/renderpass/precomputed_atmosphere_pass.h"
#include "GEngine/render_system.h"
#include "GEngine/log.h"
#include "GEngine/editor_ui.h"

#include <fstream>
#include <memory>
#include <streambuf>

/* helper funtion begin */

GLuint NewTexture2d(int width, int height) {
  GLuint texture;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  // 16F precision for the transmittance gives artifacts.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  return texture;
}

GLuint NewTexture3d(int width, int height, int depth, GLenum format,
                    bool half_precision) {
  GLuint texture;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, texture);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  GLenum internal_format = (format == GL_RGBA) ? (half_precision ? GL_RGBA16F : GL_RGBA32F)
                                               : (half_precision ? GL_RGB16F : GL_RGB32F);
  glTexImage3D(GL_TEXTURE_3D, 0, internal_format, width, height, depth, 0, format, GL_FLOAT, NULL);
  return texture;
}

bool IsFramebufferRgbFormatSupported(bool half_precision) {
  GLuint test_fbo = 0;
  glGenFramebuffers(1, &test_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, test_fbo);
  GLuint test_texture = 0;
  glGenTextures(1, &test_texture);
  glBindTexture(GL_TEXTURE_2D, test_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, half_precision ? GL_RGB16F : GL_RGB32F, 1, 1, 0, GL_RGB, GL_FLOAT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, test_texture, 0);
  bool rgb_format_supported = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  glDeleteTextures(1, &test_texture);
  glDeleteFramebuffers(1, &test_fbo);
  return rgb_format_supported;
}

double Interpolate(const std::vector<double> &wavelengths,
                   const std::vector<double> &wavelength_function,
                   double wavelength) {
  assert(wavelength_function.size() == wavelengths.size());
  if (wavelength < wavelengths[0]) {
    return wavelength_function[0];
  }
  for (unsigned int i = 0; i < wavelengths.size() - 1; ++i) {
    if (wavelength < wavelengths[i + 1]) {
      double u = (wavelength - wavelengths[i]) / (wavelengths[i + 1] - wavelengths[i]);
      return wavelength_function[i] * (1.0 - u) + wavelength_function[i + 1] * u;
    }
  }
  return wavelength_function[wavelength_function.size() - 1];
}

double CieColorMatchingFunctionTableValue(double wavelength, int column) {
  if (wavelength <= GEngine::PrecomputedAtmosphereModel::kLambdaMin || wavelength >= GEngine::PrecomputedAtmosphereModel::kLambdaMax) {
    return 0.0;
  }
  double u = (wavelength - GEngine::PrecomputedAtmosphereModel::kLambdaMin) / 5.0;
  int row = static_cast<int>(std::floor(u));
  assert(row >= 0 && row + 1 < 95);
  assert(GEngine::CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row] <= wavelength &&
         GEngine::CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1)] >= wavelength);
  u -= row;
  return GEngine::CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row + column] * (1.0 - u) +
      GEngine::CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1) + column] * u;
}

void ComputeSpectralRadianceToLuminanceFactors(
    const std::vector<double>& wavelengths,
    const std::vector<double>& solar_irradiance,
    double lambda_power, double* k_r, double* k_g, double* k_b) {
  *k_r = 0.0;
  *k_g = 0.0;
  *k_b = 0.0;
  double solar_r = Interpolate(wavelengths, solar_irradiance, GEngine::PrecomputedAtmosphereModel::kLambdaR);
  double solar_g = Interpolate(wavelengths, solar_irradiance, GEngine::PrecomputedAtmosphereModel::kLambdaG);
  double solar_b = Interpolate(wavelengths, solar_irradiance, GEngine::PrecomputedAtmosphereModel::kLambdaB);
  int dlambda = 1;
  for (int lambda = GEngine::PrecomputedAtmosphereModel::kLambdaMin; lambda < GEngine::PrecomputedAtmosphereModel::kLambdaMax; lambda += dlambda) {
    double x_bar = CieColorMatchingFunctionTableValue(lambda, 1);
    double y_bar = CieColorMatchingFunctionTableValue(lambda, 2);
    double z_bar = CieColorMatchingFunctionTableValue(lambda, 3);
    const double* xyz2srgb = GEngine::XYZ_TO_SRGB;
    double r_bar = xyz2srgb[0] * x_bar + xyz2srgb[1] * y_bar + xyz2srgb[2] * z_bar;
    double g_bar = xyz2srgb[3] * x_bar + xyz2srgb[4] * y_bar + xyz2srgb[5] * z_bar;
    double b_bar = xyz2srgb[6] * x_bar + xyz2srgb[7] * y_bar + xyz2srgb[8] * z_bar;
    double irradiance = Interpolate(wavelengths, solar_irradiance, lambda);
    *k_r += r_bar * irradiance / solar_r * pow(lambda / GEngine::PrecomputedAtmosphereModel::kLambdaR, lambda_power);
    *k_g += g_bar * irradiance / solar_g * pow(lambda / GEngine::PrecomputedAtmosphereModel::kLambdaG, lambda_power);
    *k_b += b_bar * irradiance / solar_b * pow(lambda / GEngine::PrecomputedAtmosphereModel::kLambdaB, lambda_power);
  }
  *k_r *= GEngine::MAX_LUMINOUS_EFFICACY * dlambda;
  *k_g *= GEngine::MAX_LUMINOUS_EFFICACY * dlambda;
  *k_b *= GEngine::MAX_LUMINOUS_EFFICACY * dlambda;
}

void DrawQuad(const std::vector<bool>& enable_blend, GLuint quad_vao) {
  for (unsigned int i = 0; i < enable_blend.size(); ++i) {
    if (enable_blend[i]) {
      glEnablei(GL_BLEND, i);
    }
  }

  glBindVertexArray(quad_vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  
  auto err = glGetError();
  if(err!=GL_NO_ERROR) {
    GE_WARN("gl Error {0}", err);
  }
  
  glBindVertexArray(0);

  for (unsigned int i = 0; i < enable_blend.size(); ++i) {
    glDisablei(GL_BLEND, i);
  }
}

/* helper funtion end */

/* Shader definitions begin */
namespace atmosphere {

// a basic vertex shader for a 2D FBO
const std::string kVertexShader = R"(
#version 410
layout(location = 0) in vec2 vertex;
void main() {
  gl_Position = vec4(vertex, 0.0, 1.0);
})";

// a basic geometry shader (only for 3D textures, to specify in which layer we
// want to write):
const std::string kGeometryShader = R"(
#version 410
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
uniform int layer;
void main() {
  gl_Position = gl_in[0].gl_Position;
  gl_Layer = layer;
  EmitVertex();
  gl_Position = gl_in[1].gl_Position;
  gl_Layer = layer;
  EmitVertex();
  gl_Position = gl_in[2].gl_Position;
  gl_Layer = layer;
  EmitVertex();
  EndPrimitive();
})";

// fragment shader (definitions + functions), see https://ebruneton.github.io/precomputed_atmospheric_scattering/atmosphere/functions.glsl.html
const std::string kFragmentShaderDefinitions = R"(
#define Length float
#define Wavelength float
#define Angle float
#define SolidAngle float
#define Power float
#define LuminousPower float
#define Number float
#define InverseLength float
#define Area float
#define Volume float
#define NumberDensity float
#define Irradiance float
#define Radiance float
#define SpectralPower float
#define SpectralIrradiance float
#define SpectralRadiance float
#define SpectralRadianceDensity float
#define ScatteringCoefficient float
#define InverseSolidAngle float
#define LuminousIntensity float
#define Luminance float
#define Illuminance float
#define AbstractSpectrum vec3
#define DimensionlessSpectrum vec3
#define PowerSpectrum vec3
#define IrradianceSpectrum vec3
#define RadianceSpectrum vec3
#define RadianceDensitySpectrum vec3
#define ScatteringSpectrum vec3
#define Position vec3
#define Direction vec3
#define Luminance3 vec3
#define Illuminance3 vec3
#define TransmittanceTexture sampler2D
#define AbstractScatteringTexture sampler3D
#define ReducedScatteringTexture sampler3D
#define ScatteringTexture sampler3D
#define ScatteringDensityTexture sampler3D
#define IrradianceTexture sampler2D
const Length m = 1.0;
const Wavelength nm = 1.0;
const Angle rad = 1.0;
const SolidAngle sr = 1.0;
const Power watt = 1.0;
const LuminousPower lm = 1.0;
const float PI = 3.14159265358979323846;
const Length km = 1000.0 * m;
const Area m2 = m * m;
const Volume m3 = m * m * m;
const Angle pi = PI * rad;
const Angle deg = pi / 180.0;
const Irradiance watt_per_square_meter = watt / m2;
const Radiance watt_per_square_meter_per_sr = watt / (m2 * sr);
const SpectralIrradiance watt_per_square_meter_per_nm = watt / (m2 * nm);
const SpectralRadiance watt_per_square_meter_per_sr_per_nm = watt / (m2 * sr * nm);
const SpectralRadianceDensity watt_per_cubic_meter_per_sr_per_nm = watt / (m3 * sr * nm);
const LuminousIntensity cd = lm / sr;
const LuminousIntensity kcd = 1000.0 * cd;
const Luminance cd_per_square_meter = cd / m2;
const Luminance kcd_per_square_meter = kcd / m2;

struct DensityProfileLayer {
  Length width;
  Number exp_term;
  InverseLength exp_scale;
  InverseLength linear_term;
  Number constant_term;
};
struct DensityProfile {
  DensityProfileLayer layers[2];
};
struct AtmosphereParameters {
  IrradianceSpectrum solar_irradiance;
  Angle sun_angular_radius;
  Length bottom_radius;
  Length top_radius;
  DensityProfile rayleigh_density;
  ScatteringSpectrum rayleigh_scattering;
  DensityProfile mie_density;
  ScatteringSpectrum mie_scattering;
  ScatteringSpectrum mie_extinction;
  Number mie_phase_function_g;
  DensityProfile absorption_density;
  ScatteringSpectrum absorption_extinction;
  DimensionlessSpectrum ground_albedo;
  Number mu_s_min;
};
)";

const std::string kFragmentShaderFunctions = R"(
// utility functions
Number ClampCosine(Number mu) {
  return clamp(mu, Number(-1.0), Number(1.0));
}
Length ClampDistance(Length d) {
  return max(d, 0.0 * m);
}
Length ClampRadius(IN(AtmosphereParameters) atmosphere, Length r) {
  return clamp(r, atmosphere.bottom_radius, atmosphere.top_radius);
}
Length SafeSqrt(Area a) {
  return sqrt(max(a, 0.0 * m2));
}

// Distance to the top atmosphere boundary along the view dir
Length DistanceToTopAtmosphereBoundary(IN(AtmosphereParameters) atmosphere, Length r, Number mu) {
  assert(r <= atmosphere.top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  Area discriminant = r * r * (mu * mu - 1.0) + atmosphere.top_radius * atmosphere.top_radius;
  return ClampDistance(-r * mu + SafeSqrt(discriminant));
}
// Distance to the ground along the view dir
Length DistanceToBottomAtmosphereBoundary(IN(AtmosphereParameters) atmosphere,
    Length r, Number mu) {
  assert(r >= atmosphere.bottom_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  Area discriminant = r * r * (mu * mu - 1.0) + atmosphere.bottom_radius * atmosphere.bottom_radius;
  return ClampDistance(-r * mu - SafeSqrt(discriminant));
}
// view dir intersections with the ground
bool RayIntersectsGround(IN(AtmosphereParameters) atmosphere,
    Length r, Number mu) {
  assert(r >= atmosphere.bottom_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  return mu < 0.0 && r * r * (mu * mu - 1.0) + atmosphere.bottom_radius * atmosphere.bottom_radius >= 0.0 * m2;
}

// compute the transmittance 
Number GetLayerDensity(IN(DensityProfileLayer) layer, Length altitude) {
  Number density = layer.exp_term * exp(layer.exp_scale * altitude) + layer.linear_term * altitude + layer.constant_term;
  return clamp(density, Number(0.0), Number(1.0));
}
Number GetProfileDensity(IN(DensityProfile) profile, Length altitude) {
  return altitude < profile.layers[0].width ? GetLayerDensity(profile.layers[0], altitude) : GetLayerDensity(profile.layers[1], altitude);
}
Length ComputeOpticalLengthToTopAtmosphereBoundary(IN(AtmosphereParameters) atmosphere, IN(DensityProfile) profile, Length r, Number mu) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  const int SAMPLE_COUNT = 500;
  Length dx = DistanceToTopAtmosphereBoundary(atmosphere, r, mu) / Number(SAMPLE_COUNT);
  Length result = 0.0 * m;
  for (int i = 0; i <= SAMPLE_COUNT; ++i) {
    Length d_i = Number(i) * dx;
    Length r_i = sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r);
    Number y_i = GetProfileDensity(profile, r_i - atmosphere.bottom_radius);
    Number weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
    result += y_i * weight_i * dx;
  }
  return result;
}
DimensionlessSpectrum ComputeTransmittanceToTopAtmosphereBoundary(IN(AtmosphereParameters) atmosphere, Length r, Number mu) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  return exp(-(atmosphere.rayleigh_scattering * ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.rayleigh_density, r, mu)
       + atmosphere.mie_extinction * ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.mie_density, r, mu)
       + atmosphere.absorption_extinction * ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.absorption_density, r, mu)));
}
Number GetTextureCoordFromUnitRange(Number x, int texture_size) {
  return 0.5 / Number(texture_size) + x * (1.0 - 1.0 / Number(texture_size));
}
Number GetUnitRangeFromTextureCoord(Number u, int texture_size) {
  return (u - 0.5 / Number(texture_size)) / (1.0 - 1.0 / Number(texture_size));
}
vec2 GetTransmittanceTextureUvFromRMu(IN(AtmosphereParameters) atmosphere, Length r, Number mu) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
  Length rho = SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
  Length d = DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
  Length d_min = atmosphere.top_radius - r;
  Length d_max = rho + H;
  Number x_mu = (d - d_min) / (d_max - d_min);
  Number x_r = rho / H;
  return vec2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH), GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}
void GetRMuFromTransmittanceTextureUv(IN(AtmosphereParameters) atmosphere,
    IN(vec2) uv, OUT(Length) r, OUT(Number) mu) {
  assert(uv.x >= 0.0 && uv.x <= 1.0);
  assert(uv.y >= 0.0 && uv.y <= 1.0);
  Number x_mu = GetUnitRangeFromTextureCoord(uv.x, TRANSMITTANCE_TEXTURE_WIDTH);
  Number x_r = GetUnitRangeFromTextureCoord(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT);
  Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
  Length rho = H * x_r;
  r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);
  Length d_min = atmosphere.top_radius - r;
  Length d_max = rho + H;
  Length d = d_min + x_mu * (d_max - d_min);
  mu = d == 0.0 * m ? Number(1.0) : (H * H - rho * rho - d * d) / (2.0 * r * d);
  mu = ClampCosine(mu);
}
DimensionlessSpectrum ComputeTransmittanceToTopAtmosphereBoundaryTexture(IN(AtmosphereParameters) atmosphere, IN(vec2) frag_coord) {
  const vec2 TRANSMITTANCE_TEXTURE_SIZE = vec2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
  Length r;
  Number mu;
  GetRMuFromTransmittanceTextureUv(atmosphere, frag_coord / TRANSMITTANCE_TEXTURE_SIZE, r, mu);
  return ComputeTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu);
}
DimensionlessSpectrum GetTransmittanceToTopAtmosphereBoundary(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  vec2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
  return DimensionlessSpectrum(texture(transmittance_texture, uv));
}
DimensionlessSpectrum GetTransmittance(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu, Length d, bool ray_r_mu_intersects_ground) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  assert(d >= 0.0 * m);
  Length r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
  Number mu_d = ClampCosine((r * mu + d) / r_d);
  if (ray_r_mu_intersects_ground) {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittance_texture, r_d, -mu_d) /
        GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittance_texture, r, -mu),
        DimensionlessSpectrum(1.0));
  } else {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, r, mu) /
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, r_d, mu_d),
        DimensionlessSpectrum(1.0));
  }
}
DimensionlessSpectrum GetTransmittanceToSun(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu_s) {
  Number sin_theta_h = atmosphere.bottom_radius / r;
  Number cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0));
  return GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittance_texture, r, mu_s) *
      smoothstep(-sin_theta_h * atmosphere.sun_angular_radius / rad,
                 sin_theta_h * atmosphere.sun_angular_radius / rad,
                 mu_s - cos_theta_h);
}
void ComputeSingleScatteringIntegrand(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu, Number mu_s, Number nu, Length d,
    bool ray_r_mu_intersects_ground,
    OUT(DimensionlessSpectrum) rayleigh, OUT(DimensionlessSpectrum) mie) {
  Length r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
  Number mu_s_d = ClampCosine((r * mu_s + d * nu) / r_d);
  DimensionlessSpectrum transmittance =
      GetTransmittance(
          atmosphere, transmittance_texture, r, mu, d,
          ray_r_mu_intersects_ground) *
      GetTransmittanceToSun(
          atmosphere, transmittance_texture, r_d, mu_s_d);
  rayleigh = transmittance * GetProfileDensity(
      atmosphere.rayleigh_density, r_d - atmosphere.bottom_radius);
  mie = transmittance * GetProfileDensity(
      atmosphere.mie_density, r_d - atmosphere.bottom_radius);
}
Length DistanceToNearestAtmosphereBoundary(IN(AtmosphereParameters) atmosphere,
    Length r, Number mu, bool ray_r_mu_intersects_ground) {
  if (ray_r_mu_intersects_ground) {
    return DistanceToBottomAtmosphereBoundary(atmosphere, r, mu);
  } else {
    return DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
  }
}
void ComputeSingleScattering(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    OUT(IrradianceSpectrum) rayleigh, OUT(IrradianceSpectrum) mie) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  assert(mu_s >= -1.0 && mu_s <= 1.0);
  assert(nu >= -1.0 && nu <= 1.0);
  const int SAMPLE_COUNT = 50;
  Length dx =
      DistanceToNearestAtmosphereBoundary(atmosphere, r, mu,
          ray_r_mu_intersects_ground) / Number(SAMPLE_COUNT);
  DimensionlessSpectrum rayleigh_sum = DimensionlessSpectrum(0.0);
  DimensionlessSpectrum mie_sum = DimensionlessSpectrum(0.0);
  for (int i = 0; i <= SAMPLE_COUNT; ++i) {
    Length d_i = Number(i) * dx;
    DimensionlessSpectrum rayleigh_i;
    DimensionlessSpectrum mie_i;
    ComputeSingleScatteringIntegrand(atmosphere, transmittance_texture,
        r, mu, mu_s, nu, d_i, ray_r_mu_intersects_ground, rayleigh_i, mie_i);
    Number weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
    rayleigh_sum += rayleigh_i * weight_i;
    mie_sum += mie_i * weight_i;
  }
  rayleigh = rayleigh_sum * dx * atmosphere.solar_irradiance *
      atmosphere.rayleigh_scattering;
  mie = mie_sum * dx * atmosphere.solar_irradiance * atmosphere.mie_scattering;
}
InverseSolidAngle RayleighPhaseFunction(Number nu) {
  InverseSolidAngle k = 3.0 / (16.0 * PI * sr);
  return k * (1.0 + nu * nu);
}
InverseSolidAngle MiePhaseFunction(Number g, Number nu) {
  InverseSolidAngle k = 3.0 / (8.0 * PI * sr) * (1.0 - g * g) / (2.0 + g * g);
  return k * (1.0 + nu * nu) / pow(1.0 + g * g - 2.0 * g * nu, 1.5);
}
vec4 GetScatteringTextureUvwzFromRMuMuSNu(IN(AtmosphereParameters) atmosphere,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  assert(mu_s >= -1.0 && mu_s <= 1.0);
  assert(nu >= -1.0 && nu <= 1.0);
  Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
      atmosphere.bottom_radius * atmosphere.bottom_radius);
  Length rho =
      SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
  Number u_r = GetTextureCoordFromUnitRange(rho / H, SCATTERING_TEXTURE_R_SIZE);
  Length r_mu = r * mu;
  Area discriminant =
      r_mu * r_mu - r * r + atmosphere.bottom_radius * atmosphere.bottom_radius;
  Number u_mu;
  if (ray_r_mu_intersects_ground) {
    Length d = -r_mu - SafeSqrt(discriminant);
    Length d_min = r - atmosphere.bottom_radius;
    Length d_max = rho;
    u_mu = 0.5 - 0.5 * GetTextureCoordFromUnitRange(d_max == d_min ? 0.0 :
        (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
  } else {
    Length d = -r_mu + SafeSqrt(discriminant + H * H);
    Length d_min = atmosphere.top_radius - r;
    Length d_max = rho + H;
    u_mu = 0.5 + 0.5 * GetTextureCoordFromUnitRange(
        (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
  }
  Length d = DistanceToTopAtmosphereBoundary(
      atmosphere, atmosphere.bottom_radius, mu_s);
  Length d_min = atmosphere.top_radius - atmosphere.bottom_radius;
  Length d_max = H;
  Number a = (d - d_min) / (d_max - d_min);
  Length D = DistanceToTopAtmosphereBoundary(
      atmosphere, atmosphere.bottom_radius, atmosphere.mu_s_min);
  Number A = (D - d_min) / (d_max - d_min);
  Number u_mu_s = GetTextureCoordFromUnitRange(
      max(1.0 - a / A, 0.0) / (1.0 + a), SCATTERING_TEXTURE_MU_S_SIZE);
  Number u_nu = (nu + 1.0) / 2.0;
  return vec4(u_nu, u_mu_s, u_mu, u_r);
}
void GetRMuMuSNuFromScatteringTextureUvwz(IN(AtmosphereParameters) atmosphere,
    IN(vec4) uvwz, OUT(Length) r, OUT(Number) mu, OUT(Number) mu_s,
    OUT(Number) nu, OUT(bool) ray_r_mu_intersects_ground) {
  assert(uvwz.x >= 0.0 && uvwz.x <= 1.0);
  assert(uvwz.y >= 0.0 && uvwz.y <= 1.0);
  assert(uvwz.z >= 0.0 && uvwz.z <= 1.0);
  assert(uvwz.w >= 0.0 && uvwz.w <= 1.0);
  Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
      atmosphere.bottom_radius * atmosphere.bottom_radius);
  Length rho =
      H * GetUnitRangeFromTextureCoord(uvwz.w, SCATTERING_TEXTURE_R_SIZE);
  r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);
  if (uvwz.z < 0.5) {
    Length d_min = r - atmosphere.bottom_radius;
    Length d_max = rho;
    Length d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord(
        1.0 - 2.0 * uvwz.z, SCATTERING_TEXTURE_MU_SIZE / 2);
    mu = d == 0.0 * m ? Number(-1.0) :
        ClampCosine(-(rho * rho + d * d) / (2.0 * r * d));
    ray_r_mu_intersects_ground = true;
  } else {
    Length d_min = atmosphere.top_radius - r;
    Length d_max = rho + H;
    Length d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord(
        2.0 * uvwz.z - 1.0, SCATTERING_TEXTURE_MU_SIZE / 2);
    mu = d == 0.0 * m ? Number(1.0) :
        ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d));
    ray_r_mu_intersects_ground = false;
  }
  Number x_mu_s =
      GetUnitRangeFromTextureCoord(uvwz.y, SCATTERING_TEXTURE_MU_S_SIZE);
  Length d_min = atmosphere.top_radius - atmosphere.bottom_radius;
  Length d_max = H;
  Length D = DistanceToTopAtmosphereBoundary(
      atmosphere, atmosphere.bottom_radius, atmosphere.mu_s_min);
  Number A = (D - d_min) / (d_max - d_min);
  Number a = (A - x_mu_s * A) / (1.0 + x_mu_s * A);
  Length d = d_min + min(a, A) * (d_max - d_min);
  mu_s = d == 0.0 * m ? Number(1.0) :
     ClampCosine((H * H - d * d) / (2.0 * atmosphere.bottom_radius * d));
  nu = ClampCosine(uvwz.x * 2.0 - 1.0);
}
void GetRMuMuSNuFromScatteringTextureFragCoord(
    IN(AtmosphereParameters) atmosphere, IN(vec3) frag_coord,
    OUT(Length) r, OUT(Number) mu, OUT(Number) mu_s, OUT(Number) nu,
    OUT(bool) ray_r_mu_intersects_ground) {
  const vec4 SCATTERING_TEXTURE_SIZE = vec4(
      SCATTERING_TEXTURE_NU_SIZE - 1,
      SCATTERING_TEXTURE_MU_S_SIZE,
      SCATTERING_TEXTURE_MU_SIZE,
      SCATTERING_TEXTURE_R_SIZE);
  Number frag_coord_nu =
      floor(frag_coord.x / Number(SCATTERING_TEXTURE_MU_S_SIZE));
  Number frag_coord_mu_s =
      mod(frag_coord.x, Number(SCATTERING_TEXTURE_MU_S_SIZE));
  vec4 uvwz =
      vec4(frag_coord_nu, frag_coord_mu_s, frag_coord.y, frag_coord.z) /
          SCATTERING_TEXTURE_SIZE;
  GetRMuMuSNuFromScatteringTextureUvwz(
      atmosphere, uvwz, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  nu = clamp(nu, mu * mu_s - sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)),
      mu * mu_s + sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)));
}
void ComputeSingleScatteringTexture(IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture, IN(vec3) frag_coord,
    OUT(IrradianceSpectrum) rayleigh, OUT(IrradianceSpectrum) mie) {
  Length r;
  Number mu;
  Number mu_s;
  Number nu;
  bool ray_r_mu_intersects_ground;
  GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, frag_coord,
      r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  ComputeSingleScattering(atmosphere, transmittance_texture,
      r, mu, mu_s, nu, ray_r_mu_intersects_ground, rayleigh, mie);
}
TEMPLATE(AbstractSpectrum)
AbstractSpectrum GetScattering(
    IN(AtmosphereParameters) atmosphere,
    IN(AbstractScatteringTexture TEMPLATE_ARGUMENT(AbstractSpectrum))
        scattering_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {
  vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(
      atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
  Number tex_x = floor(tex_coord_x);
  Number lerp = tex_coord_x - tex_x;
  vec3 uvw0 = vec3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  return AbstractSpectrum(texture(scattering_texture, uvw0) * (1.0 - lerp) +
      texture(scattering_texture, uvw1) * lerp);
}
RadianceSpectrum GetScattering(
    IN(AtmosphereParameters) atmosphere,
    IN(ReducedScatteringTexture) single_rayleigh_scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    IN(ScatteringTexture) multiple_scattering_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    int scattering_order) {
  if (scattering_order == 1) {
    IrradianceSpectrum rayleigh = GetScattering(
        atmosphere, single_rayleigh_scattering_texture, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    IrradianceSpectrum mie = GetScattering(
        atmosphere, single_mie_scattering_texture, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    return rayleigh * RayleighPhaseFunction(nu) +
        mie * MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
  } else {
    return GetScattering(
        atmosphere, multiple_scattering_texture, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
  }
}
IrradianceSpectrum GetIrradiance(
    IN(AtmosphereParameters) atmosphere,
    IN(IrradianceTexture) irradiance_texture,
    Length r, Number mu_s);
RadianceDensitySpectrum ComputeScatteringDensity(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    IN(ReducedScatteringTexture) single_rayleigh_scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    IN(ScatteringTexture) multiple_scattering_texture,
    IN(IrradianceTexture) irradiance_texture,
    Length r, Number mu, Number mu_s, Number nu, int scattering_order) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  assert(mu_s >= -1.0 && mu_s <= 1.0);
  assert(nu >= -1.0 && nu <= 1.0);
  assert(scattering_order >= 2);
  vec3 zenith_direction = vec3(0.0, 0.0, 1.0);
  vec3 omega = vec3(sqrt(1.0 - mu * mu), 0.0, mu);
  Number sun_dir_x = omega.x == 0.0 ? 0.0 : (nu - mu * mu_s) / omega.x;
  Number sun_dir_y = sqrt(max(1.0 - sun_dir_x * sun_dir_x - mu_s * mu_s, 0.0));
  vec3 omega_s = vec3(sun_dir_x, sun_dir_y, mu_s);
  const int SAMPLE_COUNT = 16;
  const Angle dphi = pi / Number(SAMPLE_COUNT);
  const Angle dtheta = pi / Number(SAMPLE_COUNT);
  RadianceDensitySpectrum rayleigh_mie =
      RadianceDensitySpectrum(0.0 * watt_per_cubic_meter_per_sr_per_nm);
  for (int l = 0; l < SAMPLE_COUNT; ++l) {
    Angle theta = (Number(l) + 0.5) * dtheta;
    Number cos_theta = cos(theta);
    Number sin_theta = sin(theta);
    bool ray_r_theta_intersects_ground =
        RayIntersectsGround(atmosphere, r, cos_theta);
    Length distance_to_ground = 0.0 * m;
    DimensionlessSpectrum transmittance_to_ground = DimensionlessSpectrum(0.0);
    DimensionlessSpectrum ground_albedo = DimensionlessSpectrum(0.0);
    if (ray_r_theta_intersects_ground) {
      distance_to_ground =
          DistanceToBottomAtmosphereBoundary(atmosphere, r, cos_theta);
      transmittance_to_ground =
          GetTransmittance(atmosphere, transmittance_texture, r, cos_theta,
              distance_to_ground, true /* ray_intersects_ground */);
      ground_albedo = atmosphere.ground_albedo;
    }
    for (int m = 0; m < 2 * SAMPLE_COUNT; ++m) {
      Angle phi = (Number(m) + 0.5) * dphi;
      vec3 omega_i =
          vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
      SolidAngle domega_i = (dtheta / rad) * (dphi / rad) * sin(theta) * sr;
      Number nu1 = dot(omega_s, omega_i);
      RadianceSpectrum incident_radiance = GetScattering(atmosphere,
          single_rayleigh_scattering_texture, single_mie_scattering_texture,
          multiple_scattering_texture, r, omega_i.z, mu_s, nu1,
          ray_r_theta_intersects_ground, scattering_order - 1);
      vec3 ground_normal =
          normalize(zenith_direction * r + omega_i * distance_to_ground);
      IrradianceSpectrum ground_irradiance = GetIrradiance(
          atmosphere, irradiance_texture, atmosphere.bottom_radius,
          dot(ground_normal, omega_s));
      incident_radiance += transmittance_to_ground *
          ground_albedo * (1.0 / (PI * sr)) * ground_irradiance;
      Number nu2 = dot(omega, omega_i);
      Number rayleigh_density = GetProfileDensity(
          atmosphere.rayleigh_density, r - atmosphere.bottom_radius);
      Number mie_density = GetProfileDensity(
          atmosphere.mie_density, r - atmosphere.bottom_radius);
      rayleigh_mie += incident_radiance * (
          atmosphere.rayleigh_scattering * rayleigh_density *
              RayleighPhaseFunction(nu2) +
          atmosphere.mie_scattering * mie_density *
              MiePhaseFunction(atmosphere.mie_phase_function_g, nu2)) *
          domega_i;
    }
  }
  return rayleigh_mie;
}
RadianceSpectrum ComputeMultipleScattering(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    IN(ScatteringDensityTexture) scattering_density_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  assert(mu_s >= -1.0 && mu_s <= 1.0);
  assert(nu >= -1.0 && nu <= 1.0);
  const int SAMPLE_COUNT = 50;
  Length dx =
      DistanceToNearestAtmosphereBoundary(
          atmosphere, r, mu, ray_r_mu_intersects_ground) /
              Number(SAMPLE_COUNT);
  RadianceSpectrum rayleigh_mie_sum =
      RadianceSpectrum(0.0 * watt_per_square_meter_per_sr_per_nm);
  for (int i = 0; i <= SAMPLE_COUNT; ++i) {
    Length d_i = Number(i) * dx;
    Length r_i =
        ClampRadius(atmosphere, sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r));
    Number mu_i = ClampCosine((r * mu + d_i) / r_i);
    Number mu_s_i = ClampCosine((r * mu_s + d_i * nu) / r_i);
    RadianceSpectrum rayleigh_mie_i =
        GetScattering(
            atmosphere, scattering_density_texture, r_i, mu_i, mu_s_i, nu,
            ray_r_mu_intersects_ground) *
        GetTransmittance(
            atmosphere, transmittance_texture, r, mu, d_i,
            ray_r_mu_intersects_ground) *
        dx;
    Number weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
    rayleigh_mie_sum += rayleigh_mie_i * weight_i;
  }
  return rayleigh_mie_sum;
}
RadianceDensitySpectrum ComputeScatteringDensityTexture(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    IN(ReducedScatteringTexture) single_rayleigh_scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    IN(ScatteringTexture) multiple_scattering_texture,
    IN(IrradianceTexture) irradiance_texture,
    IN(vec3) frag_coord, int scattering_order) {
  Length r;
  Number mu;
  Number mu_s;
  Number nu;
  bool ray_r_mu_intersects_ground;
  GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, frag_coord,
      r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  return ComputeScatteringDensity(atmosphere, transmittance_texture,
      single_rayleigh_scattering_texture, single_mie_scattering_texture,
      multiple_scattering_texture, irradiance_texture, r, mu, mu_s, nu,
      scattering_order);
}
RadianceSpectrum ComputeMultipleScatteringTexture(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    IN(ScatteringDensityTexture) scattering_density_texture,
    IN(vec3) frag_coord, OUT(Number) nu) {
  Length r;
  Number mu;
  Number mu_s;
  bool ray_r_mu_intersects_ground;
  GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, frag_coord,
      r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  return ComputeMultipleScattering(atmosphere, transmittance_texture,
      scattering_density_texture, r, mu, mu_s, nu,
      ray_r_mu_intersects_ground);
}
IrradianceSpectrum ComputeDirectIrradiance(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu_s) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu_s >= -1.0 && mu_s <= 1.0);
  Number alpha_s = atmosphere.sun_angular_radius / rad;
  Number average_cosine_factor =
    mu_s < -alpha_s ? 0.0 : (mu_s > alpha_s ? mu_s :
        (mu_s + alpha_s) * (mu_s + alpha_s) / (4.0 * alpha_s));
  return atmosphere.solar_irradiance *
      GetTransmittanceToTopAtmosphereBoundary(
          atmosphere, transmittance_texture, r, mu_s) * average_cosine_factor;
}
IrradianceSpectrum ComputeIndirectIrradiance(
    IN(AtmosphereParameters) atmosphere,
    IN(ReducedScatteringTexture) single_rayleigh_scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    IN(ScatteringTexture) multiple_scattering_texture,
    Length r, Number mu_s, int scattering_order) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu_s >= -1.0 && mu_s <= 1.0);
  assert(scattering_order >= 1);
  const int SAMPLE_COUNT = 32;
  const Angle dphi = pi / Number(SAMPLE_COUNT);
  const Angle dtheta = pi / Number(SAMPLE_COUNT);
  IrradianceSpectrum result =
      IrradianceSpectrum(0.0 * watt_per_square_meter_per_nm);
  vec3 omega_s = vec3(sqrt(1.0 - mu_s * mu_s), 0.0, mu_s);
  for (int j = 0; j < SAMPLE_COUNT / 2; ++j) {
    Angle theta = (Number(j) + 0.5) * dtheta;
    for (int i = 0; i < 2 * SAMPLE_COUNT; ++i) {
      Angle phi = (Number(i) + 0.5) * dphi;
      vec3 omega =
          vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
      SolidAngle domega = (dtheta / rad) * (dphi / rad) * sin(theta) * sr;
      Number nu = dot(omega, omega_s);
      result += GetScattering(atmosphere, single_rayleigh_scattering_texture,
          single_mie_scattering_texture, multiple_scattering_texture,
          r, omega.z, mu_s, nu, false /* ray_r_theta_intersects_ground */,
          scattering_order) *
              omega.z * domega;
    }
  }
  return result;
}
vec2 GetIrradianceTextureUvFromRMuS(IN(AtmosphereParameters) atmosphere,
    Length r, Number mu_s) {
  assert(r >= atmosphere.bottom_radius && r <= atmosphere.top_radius);
  assert(mu_s >= -1.0 && mu_s <= 1.0);
  Number x_r = (r - atmosphere.bottom_radius) /
      (atmosphere.top_radius - atmosphere.bottom_radius);
  Number x_mu_s = mu_s * 0.5 + 0.5;
  return vec2(GetTextureCoordFromUnitRange(x_mu_s, IRRADIANCE_TEXTURE_WIDTH),
              GetTextureCoordFromUnitRange(x_r, IRRADIANCE_TEXTURE_HEIGHT));
}
void GetRMuSFromIrradianceTextureUv(IN(AtmosphereParameters) atmosphere,
    IN(vec2) uv, OUT(Length) r, OUT(Number) mu_s) {
  assert(uv.x >= 0.0 && uv.x <= 1.0);
  assert(uv.y >= 0.0 && uv.y <= 1.0);
  Number x_mu_s = GetUnitRangeFromTextureCoord(uv.x, IRRADIANCE_TEXTURE_WIDTH);
  Number x_r = GetUnitRangeFromTextureCoord(uv.y, IRRADIANCE_TEXTURE_HEIGHT);
  r = atmosphere.bottom_radius +
      x_r * (atmosphere.top_radius - atmosphere.bottom_radius);
  mu_s = ClampCosine(2.0 * x_mu_s - 1.0);
}
const vec2 IRRADIANCE_TEXTURE_SIZE =
    vec2(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
IrradianceSpectrum ComputeDirectIrradianceTexture(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    IN(vec2) frag_coord) {
  Length r;
  Number mu_s;
  GetRMuSFromIrradianceTextureUv(atmosphere, frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s);
  return ComputeDirectIrradiance(atmosphere, transmittance_texture, r, mu_s);
}
IrradianceSpectrum ComputeIndirectIrradianceTexture(
    IN(AtmosphereParameters) atmosphere,
    IN(ReducedScatteringTexture) single_rayleigh_scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    IN(ScatteringTexture) multiple_scattering_texture,
    IN(vec2) frag_coord, int scattering_order) {
  Length r;
  Number mu_s;
  GetRMuSFromIrradianceTextureUv(
      atmosphere, frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s);
  return ComputeIndirectIrradiance(atmosphere,
      single_rayleigh_scattering_texture, single_mie_scattering_texture,
      multiple_scattering_texture, r, mu_s, scattering_order);
}
IrradianceSpectrum GetIrradiance(
    IN(AtmosphereParameters) atmosphere,
    IN(IrradianceTexture) irradiance_texture,
    Length r, Number mu_s) {
  vec2 uv = GetIrradianceTextureUvFromRMuS(atmosphere, r, mu_s);
  return IrradianceSpectrum(texture(irradiance_texture, uv));
}
#ifdef COMBINED_SCATTERING_TEXTURES
vec3 GetExtrapolatedSingleMieScattering(
    IN(AtmosphereParameters) atmosphere, IN(vec4) scattering) {
  if (scattering.r <= 0.0) {
    return vec3(0.0);
  }
  return scattering.rgb * scattering.a / scattering.r *
	    (atmosphere.rayleigh_scattering.r / atmosphere.mie_scattering.r) *
	    (atmosphere.mie_scattering / atmosphere.rayleigh_scattering);
}
#endif
IrradianceSpectrum GetCombinedScattering(
    IN(AtmosphereParameters) atmosphere,
    IN(ReducedScatteringTexture) scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    OUT(IrradianceSpectrum) single_mie_scattering) {
  vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(
      atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
  Number tex_x = floor(tex_coord_x);
  Number lerp = tex_coord_x - tex_x;
  vec3 uvw0 = vec3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
#ifdef COMBINED_SCATTERING_TEXTURES
  vec4 combined_scattering =
      texture(scattering_texture, uvw0) * (1.0 - lerp) +
      texture(scattering_texture, uvw1) * lerp;
  IrradianceSpectrum scattering = IrradianceSpectrum(combined_scattering);
  single_mie_scattering =
      GetExtrapolatedSingleMieScattering(atmosphere, combined_scattering);
#else
  IrradianceSpectrum scattering = IrradianceSpectrum(
      texture(scattering_texture, uvw0) * (1.0 - lerp) +
      texture(scattering_texture, uvw1) * lerp);
  single_mie_scattering = IrradianceSpectrum(
      texture(single_mie_scattering_texture, uvw0) * (1.0 - lerp) +
      texture(single_mie_scattering_texture, uvw1) * lerp);
#endif
  return scattering;
}
RadianceSpectrum GetSkyRadiance(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    IN(ReducedScatteringTexture) scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    Position camera, IN(Direction) view_ray, Length shadow_length,
    IN(Direction) sun_direction, OUT(DimensionlessSpectrum) transmittance) {
  Length r = length(camera);
  Length rmu = dot(camera, view_ray);
  Length distance_to_top_atmosphere_boundary = -rmu -
      sqrt(rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius);
  if (distance_to_top_atmosphere_boundary > 0.0 * m) {
    camera = camera + view_ray * distance_to_top_atmosphere_boundary;
    r = atmosphere.top_radius;
    rmu += distance_to_top_atmosphere_boundary;
  } else if (r > atmosphere.top_radius) {
    transmittance = DimensionlessSpectrum(1.0);
    return RadianceSpectrum(0.0 * watt_per_square_meter_per_sr_per_nm);
  }
  Number mu = rmu / r;
  Number mu_s = dot(camera, sun_direction) / r;
  Number nu = dot(view_ray, sun_direction);
  bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);
  transmittance = ray_r_mu_intersects_ground ? DimensionlessSpectrum(0.0) :
      GetTransmittanceToTopAtmosphereBoundary(
          atmosphere, transmittance_texture, r, mu);
  IrradianceSpectrum single_mie_scattering;
  IrradianceSpectrum scattering;
  if (shadow_length == 0.0 * m) {
    scattering = GetCombinedScattering(
        atmosphere, scattering_texture, single_mie_scattering_texture,
        r, mu, mu_s, nu, ray_r_mu_intersects_ground,
        single_mie_scattering);
  } else {
    Length d = shadow_length;
    Length r_p =
        ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
    Number mu_p = (r * mu + d) / r_p;
    Number mu_s_p = (r * mu_s + d * nu) / r_p;
    scattering = GetCombinedScattering(
        atmosphere, scattering_texture, single_mie_scattering_texture,
        r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
        single_mie_scattering);
    DimensionlessSpectrum shadow_transmittance =
        GetTransmittance(atmosphere, transmittance_texture,
            r, mu, shadow_length, ray_r_mu_intersects_ground);
    scattering = scattering * shadow_transmittance;
    single_mie_scattering = single_mie_scattering * shadow_transmittance;
  }
  return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
      MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
}
RadianceSpectrum GetSkyRadianceToPoint(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    IN(ReducedScatteringTexture) scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    Position camera, IN(Position) point, Length shadow_length,
    IN(Direction) sun_direction, OUT(DimensionlessSpectrum) transmittance) {
  Direction view_ray = normalize(point - camera);
  Length r = length(camera);
  Length rmu = dot(camera, view_ray);
  Length distance_to_top_atmosphere_boundary = -rmu -
      sqrt(rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius);
  if (distance_to_top_atmosphere_boundary > 0.0 * m) {
    camera = camera + view_ray * distance_to_top_atmosphere_boundary;
    r = atmosphere.top_radius;
    rmu += distance_to_top_atmosphere_boundary;
  }
  Number mu = rmu / r;
  Number mu_s = dot(camera, sun_direction) / r;
  Number nu = dot(view_ray, sun_direction);
  Length d = length(point - camera);
  bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);
  transmittance = GetTransmittance(atmosphere, transmittance_texture,
      r, mu, d, ray_r_mu_intersects_ground);
  IrradianceSpectrum single_mie_scattering;
  IrradianceSpectrum scattering = GetCombinedScattering(
      atmosphere, scattering_texture, single_mie_scattering_texture,
      r, mu, mu_s, nu, ray_r_mu_intersects_ground,
      single_mie_scattering);
  d = max(d - shadow_length, 0.0 * m);
  Length r_p = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
  Number mu_p = (r * mu + d) / r_p;
  Number mu_s_p = (r * mu_s + d * nu) / r_p;
  IrradianceSpectrum single_mie_scattering_p;
  IrradianceSpectrum scattering_p = GetCombinedScattering(
      atmosphere, scattering_texture, single_mie_scattering_texture,
      r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
      single_mie_scattering_p);
  DimensionlessSpectrum shadow_transmittance = transmittance;
  if (shadow_length > 0.0 * m) {
    shadow_transmittance = GetTransmittance(atmosphere, transmittance_texture,
        r, mu, d, ray_r_mu_intersects_ground);
  }
  scattering = scattering - shadow_transmittance * scattering_p;
  single_mie_scattering =
      single_mie_scattering - shadow_transmittance * single_mie_scattering_p;
#ifdef COMBINED_SCATTERING_TEXTURES
  single_mie_scattering = GetExtrapolatedSingleMieScattering(
      atmosphere, vec4(scattering, single_mie_scattering.r));
#endif
  single_mie_scattering = single_mie_scattering *
      smoothstep(Number(0.0), Number(0.01), mu_s);
  return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
      MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
}
IrradianceSpectrum GetSunAndSkyIrradiance(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    IN(IrradianceTexture) irradiance_texture,
    IN(Position) point, IN(Direction) normal, IN(Direction) sun_direction,
    OUT(IrradianceSpectrum) sky_irradiance) {
  Length r = length(point);
  Number mu_s = dot(point, sun_direction) / r;
  sky_irradiance = GetIrradiance(atmosphere, irradiance_texture, r, mu_s) *
      (1.0 + dot(normal, point) / r) * 0.5;
  return atmosphere.solar_irradiance *
      GetTransmittanceToSun(
          atmosphere, transmittance_texture, r, mu_s) *
      max(dot(normal, sun_direction), 0.0);
}
)";

const char kDemoVertexShader[] = R"(
#version 410
uniform mat4 model_from_view;
uniform mat4 view_from_clip;
layout(location = 0) in vec4 vertex;
out vec3 view_ray;
void main() {
  view_ray = (model_from_view * vec4((view_from_clip * vertex).xyz, 0.0)).xyz;
  gl_Position = vertex;
})";

#include "demo.glsl.inc"

/* Shader definitions end */

/* precompute textures shaders begin */

/**
 * Note that these shaders must be concatenated with kFragmentShaderDefinitions and kFragmentShaderFunctions,
 * as well as with a definition of the ATMOSPHERE constant
 */
const std::string kComputeTransmittanceShader = R"(
    layout(location = 0) out vec3 transmittance;
    void main() {
      transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(ATMOSPHERE, gl_FragCoord.xy);
    })";

const std::string kComputeDirectIrradianceShader = R"(
    layout(location = 0) out vec3 delta_irradiance;
    layout(location = 1) out vec3 irradiance;
    uniform sampler2D transmittance_texture;
    void main() {
      delta_irradiance = ComputeDirectIrradianceTexture(ATMOSPHERE, transmittance_texture, gl_FragCoord.xy);
      irradiance = vec3(0.0);
    })";
  
  const std::string kComputeSingleScatteringShader = R"(
    layout(location = 0) out vec3 delta_rayleigh;
    layout(location = 1) out vec3 delta_mie;
    layout(location = 2) out vec4 scattering;
    layout(location = 3) out vec3 single_mie_scattering;
    uniform mat3 luminance_from_radiance;
    uniform sampler2D transmittance_texture;
    uniform int layer;
    void main() {
      ComputeSingleScatteringTexture(ATMOSPHERE, transmittance_texture, vec3(gl_FragCoord.xy, layer + 0.5), delta_rayleigh, delta_mie);
      scattering = vec4(luminance_from_radiance * delta_rayleigh.rgb, (luminance_from_radiance * delta_mie).r);
      single_mie_scattering = luminance_from_radiance * delta_mie;
    })";

const std::string kComputeScatteringDensityShader = R"(
    layout(location = 0) out vec3 scattering_density;
    uniform sampler2D transmittance_texture;
    uniform sampler3D single_rayleigh_scattering_texture;
    uniform sampler3D single_mie_scattering_texture;
    uniform sampler3D multiple_scattering_texture;
    uniform sampler2D irradiance_texture;
    uniform int scattering_order;
    uniform int layer;
    void main() {
      scattering_density = ComputeScatteringDensityTexture(
          ATMOSPHERE, transmittance_texture,
          single_rayleigh_scattering_texture,
          single_mie_scattering_texture,
          multiple_scattering_texture,
          irradiance_texture,
          vec3(gl_FragCoord.xy, layer + 0.5),
          scattering_order);
    })";

const std::string kComputeIndirectIrradianceShader = R"(
    layout(location = 0) out vec3 delta_irradiance;
    layout(location = 1) out vec3 irradiance;
    uniform mat3 luminance_from_radiance;
    uniform sampler3D single_rayleigh_scattering_texture;
    uniform sampler3D single_mie_scattering_texture;
    uniform sampler3D multiple_scattering_texture;
    uniform int scattering_order;
    void main() {
      delta_irradiance = ComputeIndirectIrradianceTexture(
          ATMOSPHERE,
          single_rayleigh_scattering_texture,
          single_mie_scattering_texture,
          multiple_scattering_texture,
          gl_FragCoord.xy,
          scattering_order);
      irradiance = luminance_from_radiance * delta_irradiance;
    })";

const std::string kComputeMultipleScatteringShader = R"(
    layout(location = 0) out vec3 delta_multiple_scattering;
    layout(location = 1) out vec4 scattering;
    uniform mat3 luminance_from_radiance;
    uniform sampler2D transmittance_texture;
    uniform sampler3D scattering_density_texture;
    uniform int layer;
    void main() {
      float nu;
      delta_multiple_scattering = ComputeMultipleScatteringTexture(
          ATMOSPHERE, transmittance_texture,
          scattering_density_texture,
          vec3(gl_FragCoord.xy, layer + 0.5), nu);
      scattering = vec4(luminance_from_radiance * delta_multiple_scattering.rgb / RayleighPhaseFunction(nu), 0.0);
    })";


/* precompute textures shaders end */

/* kAtmosphereShader which exposed our API beign */

const std::string kAtmosphereShader = R"(
    uniform sampler2D transmittance_texture;
    uniform sampler3D scattering_texture;
    uniform sampler3D single_mie_scattering_texture;
    uniform sampler2D irradiance_texture;
    #ifdef RADIANCE_API_ENABLED
    RadianceSpectrum GetSolarRadiance() {
      return ATMOSPHERE.solar_irradiance /
          (PI * ATMOSPHERE.sun_angular_radius * ATMOSPHERE.sun_angular_radius);
    }
    RadianceSpectrum GetSkyRadiance(
        Position camera, Direction view_ray, Length shadow_length,
        Direction sun_direction, out DimensionlessSpectrum transmittance) {
      return GetSkyRadiance(ATMOSPHERE, transmittance_texture,
          scattering_texture, single_mie_scattering_texture,
          camera, view_ray, shadow_length, sun_direction, transmittance);
    }
    RadianceSpectrum GetSkyRadianceToPoint(
        Position camera, Position point, Length shadow_length,
        Direction sun_direction, out DimensionlessSpectrum transmittance) {
      return GetSkyRadianceToPoint(ATMOSPHERE, transmittance_texture,
          scattering_texture, single_mie_scattering_texture,
          camera, point, shadow_length, sun_direction, transmittance);
    }
    IrradianceSpectrum GetSunAndSkyIrradiance(
       Position p, Direction normal, Direction sun_direction,
       out IrradianceSpectrum sky_irradiance) {
      return GetSunAndSkyIrradiance(ATMOSPHERE, transmittance_texture,
          irradiance_texture, p, normal, sun_direction, sky_irradiance);
    }
    #endif
    Luminance3 GetSolarLuminance() {
      return ATMOSPHERE.solar_irradiance /
          (PI * ATMOSPHERE.sun_angular_radius * ATMOSPHERE.sun_angular_radius) *
          SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
    }
    Luminance3 GetSkyLuminance(
        Position camera, Direction view_ray, Length shadow_length,
        Direction sun_direction, out DimensionlessSpectrum transmittance) {
      return GetSkyRadiance(ATMOSPHERE, transmittance_texture,
          scattering_texture, single_mie_scattering_texture,
          camera, view_ray, shadow_length, sun_direction, transmittance) *
          SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
    }
    Luminance3 GetSkyLuminanceToPoint(
        Position camera, Position point, Length shadow_length,
        Direction sun_direction, out DimensionlessSpectrum transmittance) {
      return GetSkyRadianceToPoint(ATMOSPHERE, transmittance_texture,
          scattering_texture, single_mie_scattering_texture,
          camera, point, shadow_length, sun_direction, transmittance) *
          SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
    }
    Illuminance3 GetSunAndSkyIlluminance(
       Position p, Direction normal, Direction sun_direction,
       out IrradianceSpectrum sky_irradiance) {
      IrradianceSpectrum sun_irradiance = GetSunAndSkyIrradiance(
          ATMOSPHERE, transmittance_texture, irradiance_texture, p, normal,
          sun_direction, sky_irradiance);
      sky_irradiance *= SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
      return sun_irradiance * SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
    })";

/* kAtmosphereShader which exposed our API end */

} // namespace atmosphere

GEngine::PrecomputedAtmospherePass::PrecomputedAtmospherePass(const std::string &name, int order)
    : CRenderPass(name, order) {
}

GEngine::PrecomputedAtmospherePass::~PrecomputedAtmospherePass() {}

void GEngine::PrecomputedAtmospherePass::Init() {
  constexpr int kLambdaMin = 360;
  constexpr int kLambdaMax = 830;
  constexpr double kSolarIrradiance[48] = {
      1.11776, 1.14259, 1.01249, 1.14716, 1.72765, 1.73054, 1.6887,  1.61253,
      1.91198, 2.03474, 2.02042, 2.02212, 1.93377, 1.95809, 1.91686, 1.8298,
      1.8685,  1.8931,  1.85149, 1.8504,  1.8341,  1.8345,  1.8147,  1.78158,
      1.7533,  1.6965,  1.68194, 1.64654, 1.6048,  1.52143, 1.55622, 1.5113,
      1.474,   1.4482,  1.41018, 1.36775, 1.34188, 1.31429, 1.28303, 1.26758,
      1.2367,  1.2082,  1.18737, 1.14683, 1.12362, 1.1058,  1.07124, 1.04992};
  constexpr double kOzoneCrossSection[48] = {
      1.18e-27,  2.182e-28, 2.818e-28, 6.636e-28, 1.527e-27, 2.763e-27,
      5.52e-27,  8.451e-27, 1.582e-26, 2.316e-26, 3.669e-26, 4.924e-26,
      7.752e-26, 9.016e-26, 1.48e-25,  1.602e-25, 2.139e-25, 2.755e-25,
      3.091e-25, 3.5e-25,   4.266e-25, 4.672e-25, 4.398e-25, 4.701e-25,
      5.019e-25, 4.305e-25, 3.74e-25,  3.215e-25, 2.662e-25, 2.238e-25,
      1.852e-25, 1.473e-25, 1.209e-25, 9.423e-26, 7.455e-26, 6.566e-26,
      5.105e-26, 4.15e-26,  4.228e-26, 3.237e-26, 2.451e-26, 2.801e-26,
      2.534e-26, 1.624e-26, 1.465e-26, 2.078e-26, 1.383e-26, 7.105e-27};
  constexpr double kDobsonUnit = 2.687e20;
  constexpr double kMaxOzoneNumberDensity = 300.0 * kDobsonUnit / 15000.0;
  constexpr double kConstantSolarIrradiance = 1.5;
  constexpr double kBottomRadius = 6360000.0;
  constexpr double kTopRadius = 6420000.0;
  constexpr double kRayleigh = 1.24062e-6;
  constexpr double kRayleighScaleHeight = 8000.0;
  constexpr double kMieScaleHeight = 1200.0;
  constexpr double kMieAngstromAlpha = 0.0;
  constexpr double kMieAngstromBeta = 5.328e-3;
  constexpr double kMieSingleScatteringAlbedo = 0.9;
  constexpr double kMiePhaseFunctionG = 0.8;
  constexpr double kGroundAlbedo = 0.1;
  const double max_sun_zenith_angle =
      (use_half_precision_ ? 102.0 : 120.0) / 180.0 * kPi;

  // set up layers
  DensityProfileLayer rayleigh_layer(0.0, 1.0, -1.0 / kRayleighScaleHeight, 0.0, 0.0);
  DensityProfileLayer mie_layer(0.0, 1.0, -1.0 / kMieScaleHeight, 0.0, 0.0);
  std::vector<DensityProfileLayer> ozone_density;
  ozone_density.push_back(DensityProfileLayer(25000.0, 0.0, 0.0, 1.0 / 15000.0, -2.0 / 3.0));
  ozone_density.push_back(DensityProfileLayer(0.0, 0.0, 0.0, -1.0 / 15000.0, 8.0 / 3.0));

  // scattering parameters of wavelength from 360nm to 830nm
  std::vector<double> wavelengths;
  std::vector<double> solar_irradiance;
  std::vector<double> rayleigh_scattering;
  std::vector<double> mie_scattering;
  std::vector<double> mie_extinction;
  std::vector<double> absorption_extinction;
  std::vector<double> ground_albedo;
  for (int l = kLambdaMin; l <= kLambdaMax; l += 10) {
    double lambda = static_cast<double>(l) * 1e-3; // micro-meters
    double mie = kMieAngstromBeta / kMieScaleHeight * pow(lambda, -kMieAngstromAlpha);
    wavelengths.push_back(l);
    if (use_constant_solar_spectrum_) {
      solar_irradiance.push_back(kConstantSolarIrradiance);
    } else {
      solar_irradiance.push_back(kSolarIrradiance[(l - kLambdaMin) / 10]);
    }
    rayleigh_scattering.push_back(kRayleigh * pow(lambda, -4));
    mie_scattering.push_back(mie * kMieSingleScatteringAlbedo);
    mie_extinction.push_back(mie);
    absorption_extinction.push_back(use_ozone_ ? kMaxOzoneNumberDensity * kOzoneCrossSection[(l - kLambdaMin) / 10] : 0.0);
    ground_albedo.push_back(kGroundAlbedo);
  }
  
  // full_screen_quad_vao_ for pass
  glGenVertexArrays(1, &full_screen_quad_vao_);
  glBindVertexArray(full_screen_quad_vao_);
  glGenBuffers(1, &full_screen_quad_vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, full_screen_quad_vbo_);
  const float vertices[] = {
    -1.0, -1.0, 0.0, 1.0,
    +1.0, -1.0, 0.0, 1.0,
    -1.0, +1.0, 0.0, 1.0,
    +1.0, +1.0, 0.0, 1.0,
  };
  glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), vertices, GL_STATIC_DRAW);
  constexpr GLuint kAttribIndex = 0;
  constexpr int kCoordsPerVertex = 4;
  glVertexAttribPointer(kAttribIndex, kCoordsPerVertex, GL_FLOAT, false, 0, (void*)0);
  glEnableVertexAttribArray(kAttribIndex);
  glBindVertexArray(0);

  // precompute the transmittance, scattering and irradiance textures,
  // impletation of Algorithm 4.1 of the original paper
  model_.reset(new PrecomputedAtmosphereModel(
      wavelengths, solar_irradiance, kSunAngularRadius, kBottomRadius,
      kTopRadius, {rayleigh_layer}, rayleigh_scattering, {mie_layer},
      mie_scattering, mie_extinction, kMiePhaseFunctionG, ozone_density,
      absorption_extinction, ground_albedo, max_sun_zenith_angle,
      kLengthUnitInMeters, use_luminance_ == PRECOMPUTED ? 15 : 3,
      use_combined_textures_, use_half_precision_));
  model_->Init();

  // create shader for demo scene
  const std::string fragment_shader_str =
      "#version 410\n" +
      std::string(use_luminance_ != NONE ? "#define USE_LUMINANCE\n" : "") +
      "const float kLengthUnitInMeters = " +
      std::to_string(kLengthUnitInMeters) + ";\n" +
      atmosphere::demo_glsl;

  // program_ = Shader::CreateProgramFromSource(atmosphere::kDemoVertexShader, fragment_shader_str);
  program_ = Shader::CreateAtmosphereProgram("/Users/huzhihao/Code/Engine/myRenderer/GEngine/src/GEngine/renderpass/precomputed_atmosphere_vert.glsl", "/Users/huzhihao/Code/Engine/myRenderer/GEngine/src/GEngine/renderpass/precomputed_atmosphere_frag.glsl", "/Users/huzhihao/Code/Engine/myRenderer/GEngine/src/GEngine/renderpass/atmosphere_shader_frag2.glsl");

  glUseProgram(program_->GetShaderID());
//  model_->SetProgramUniforms(program_->GetShaderID(), 0, 1, 2);
  model_->SetProgramUniforms(program_->GetShaderID(), 0, 1, 2, 3);

  double white_point_r = 1.0;
  double white_point_g = 1.0;
  double white_point_b = 1.0;

  if (do_white_balance_) {
    PrecomputedAtmosphereModel::ConvertSpectrumToLinearSrgb(wavelengths, solar_irradiance, &white_point_r, &white_point_g, &white_point_b);
    double white_point = (white_point_r + white_point_g + white_point_b) / 3.0;
    white_point_r /= white_point;
    white_point_g /= white_point;
    white_point_b /= white_point;
  }

  glUniform3f(glGetUniformLocation(program_->GetShaderID(), "white_point"), white_point_r, white_point_g, white_point_b);
  glUniform3f(glGetUniformLocation(program_->GetShaderID(), "earth_center"), 0.0, 0.0, -kBottomRadius / kLengthUnitInMeters);
  glUniform2f(glGetUniformLocation(program_->GetShaderID(), "sun_size"), tan(kSunAngularRadius), cos(kSunAngularRadius));

  auto viewport_width = static_cast<float>(GEngine::WINDOW_CONFIG::VIEWPORT_WIDTH);
  auto viewport_height = static_cast<float>(GEngine::WINDOW_CONFIG::VIEWPORT_HEIGHT);
  glViewport(0, 0, viewport_width, viewport_height);

  const float kFovY = 50.0 / 180.0 * kPi;
  const float kTanFovY = tan(kFovY / 2.0);
  float aspect_ratio = static_cast<float>(viewport_width) / viewport_height;

  // Transform matrix from clip space to camera space (i.e. the inverse of a
  // GL_PROJECTION matrix).
  float view_from_clip[16] = {
    kTanFovY * aspect_ratio, 0.0, 0.0, 0.0,
    0.0, kTanFovY, 0.0, 0.0,
    0.0, 0.0, 0.0, -1.0,
    0.0, 0.0, 1.0, 1.0
  };
  glUniformMatrix4fv(glGetUniformLocation(program_->GetShaderID(), "view_from_clip"), 1, true, view_from_clip);
}

void GEngine::PrecomputedAtmospherePass::Tick() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(program_->GetShaderID());

  view_distance_meters_ = CSingleton<CRenderSystem>()->GetOrCreateMainUI()->distance_;
  view_zenith_angle_radians_ = CSingleton<CRenderSystem>()->GetOrCreateMainUI()->view_angle_[0];
  view_azimuth_angle_radians_ = CSingleton<CRenderSystem>()->GetOrCreateMainUI()->view_angle_[1];
  sun_zenith_angle_radians_ = CSingleton<CRenderSystem>()->GetOrCreateMainUI()->sun_angle_[0];
  sun_azimuth_angle_radians_ = CSingleton<CRenderSystem>()->GetOrCreateMainUI()->sun_angle_[1];
  exposure_ = 10.0;

  glUniform1i(glGetUniformLocation(program_->GetShaderID(), "u_level"), CSingleton<CRenderSystem>()->GetOrCreateMainUI()->texture_level_);
  glUniform1i(glGetUniformLocation(program_->GetShaderID(), "u_display_content"), CSingleton<CRenderSystem>()->GetOrCreateMainUI()->display_content_);

  float cos_z = cos(view_zenith_angle_radians_);
  float sin_z = sin(view_zenith_angle_radians_);
  float cos_a = cos(view_azimuth_angle_radians_);
  float sin_a = sin(view_azimuth_angle_radians_);
  float ux[3] = { -sin_a, cos_a, 0.0 };
  float uy[3] = { -cos_z * cos_a, -cos_z * sin_a, sin_z };
  float uz[3] = { sin_z * cos_a, sin_z * sin_a, cos_z };
  float l = view_distance_meters_ / kLengthUnitInMeters;

  // Transform matrix from camera frame to world space (i.e. the inverse of a GL_MODELVIEW matrix).
  float model_from_view[16] = {
    ux[0], uy[0], uz[0], uz[0] * l,
    ux[1], uy[1], uz[1], uz[1] * l,
    ux[2], uy[2], uz[2], uz[2] * l,
    0.0, 0.0, 0.0, 1.0
  };

  glUniform3f(glGetUniformLocation(program_->GetShaderID(), "camera"), model_from_view[3], model_from_view[7], model_from_view[11]);
  glUniform1f(glGetUniformLocation(program_->GetShaderID(), "exposure"), use_luminance_ != NONE ? exposure_ * 1e-5 : exposure_);
  glUniformMatrix4fv(glGetUniformLocation(program_->GetShaderID(), "model_from_view"), 1, true, model_from_view);
  glUniform3f(glGetUniformLocation(program_->GetShaderID(), "sun_direction"),
      cos(sun_azimuth_angle_radians_) * sin(sun_zenith_angle_radians_),
      sin(sun_azimuth_angle_radians_) * sin(sun_zenith_angle_radians_),
      cos(sun_zenith_angle_radians_));
  
  glBindVertexArray(full_screen_quad_vao_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

// The constructor of the PrecomputedAtmosphereModel class allocates the
// precomputed textures, but does not initialize them.
GEngine::PrecomputedAtmosphereModel::PrecomputedAtmosphereModel(
    const std::vector<double> &wavelengths,
    const std::vector<double> &solar_irradiance, double sun_angular_radius,
    double bottom_radius, double top_radius,
    const std::vector<DensityProfileLayer> &rayleigh_density,
    const std::vector<double> &rayleigh_scattering,
    const std::vector<DensityProfileLayer> &mie_density,
    const std::vector<double> &mie_scattering,
    const std::vector<double> &mie_extinction, double mie_phase_function_g,
    const std::vector<DensityProfileLayer> &absorption_density,
    const std::vector<double> &absorption_extinction,
    const std::vector<double> &ground_albedo, double max_sun_zenith_angle,
    double length_unit_in_meters, unsigned int num_precomputed_wavelengths,
    bool combine_scattering_textures, bool half_precision)
    : num_precomputed_wavelengths_(num_precomputed_wavelengths),
      half_precision_(half_precision),
      rgb_format_supported_(IsFramebufferRgbFormatSupported(half_precision)) {
  // prepare shader datas
  auto to_string = [&wavelengths](const std::vector<double> &v, const vec3 &lambdas, double scale) {
    double r = Interpolate(wavelengths, v, lambdas[0]) * scale;
    double g = Interpolate(wavelengths, v, lambdas[1]) * scale;
    double b = Interpolate(wavelengths, v, lambdas[2]) * scale;
    return "vec3(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
  };
  auto density_layer =
      [length_unit_in_meters](const DensityProfileLayer &layer) {
        return "DensityProfileLayer(" +
               std::to_string(layer.width / length_unit_in_meters) + "," +
               std::to_string(layer.exp_term) + "," +
               std::to_string(layer.exp_scale * length_unit_in_meters) + "," +
               std::to_string(layer.linear_term * length_unit_in_meters) + "," +
               std::to_string(layer.constant_term) + ")";
      };
  auto density_profile =
      [density_layer](std::vector<DensityProfileLayer> layers) {
        constexpr int kLayerCount = 2;
        while (layers.size() < kLayerCount) {
          layers.insert(layers.begin(), DensityProfileLayer());
        }
        std::string result = "DensityProfile(DensityProfileLayer[" + std::to_string(kLayerCount) + "](";
        for (int i = 0; i < kLayerCount; ++i) {
          result += density_layer(layers[i]);
          result += i < kLayerCount - 1 ? "," : "))";
        }
        return result;
      };
  bool precompute_illuminance = num_precomputed_wavelengths > 3;
  double sky_k_r, sky_k_g, sky_k_b;
  if (precompute_illuminance) {
    sky_k_r = sky_k_g = sky_k_b = MAX_LUMINOUS_EFFICACY;
  } else {
    ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance, -3 /* lambda_power */,
                                              &sky_k_r, &sky_k_g, &sky_k_b);
  }
  double sun_k_r, sun_k_g, sun_k_b;
  ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance, 0 /* lambda_power */, 
                                            &sun_k_r, &sun_k_g, &sun_k_b);

  glsl_header_factory_ = [=](const vec3& lambdas) {
    return
      "#version 410\n"
      "#define IN(x) const in x\n"
      "#define OUT(x) out x\n"
      "#define TEMPLATE(x)\n"
      "#define TEMPLATE_ARGUMENT(x)\n"
      "#define assert(x)\n"
      "const int TRANSMITTANCE_TEXTURE_WIDTH = " +
          std::to_string(TRANSMITTANCE_TEXTURE_WIDTH) + ";\n" +
      "const int TRANSMITTANCE_TEXTURE_HEIGHT = " +
          std::to_string(TRANSMITTANCE_TEXTURE_HEIGHT) + ";\n" +
      "const int SCATTERING_TEXTURE_R_SIZE = " +
          std::to_string(SCATTERING_TEXTURE_R_SIZE) + ";\n" +
      "const int SCATTERING_TEXTURE_MU_SIZE = " +
          std::to_string(SCATTERING_TEXTURE_MU_SIZE) + ";\n" +
      "const int SCATTERING_TEXTURE_MU_S_SIZE = " +
          std::to_string(SCATTERING_TEXTURE_MU_S_SIZE) + ";\n" +
      "const int SCATTERING_TEXTURE_NU_SIZE = " +
          std::to_string(SCATTERING_TEXTURE_NU_SIZE) + ";\n" +
      "const int IRRADIANCE_TEXTURE_WIDTH = " +
          std::to_string(IRRADIANCE_TEXTURE_WIDTH) + ";\n" +
      "const int IRRADIANCE_TEXTURE_HEIGHT = " +
          std::to_string(IRRADIANCE_TEXTURE_HEIGHT) + ";\n" +
      (combine_scattering_textures ?
          "#define COMBINED_SCATTERING_TEXTURES\n" : "") +
      atmosphere::kFragmentShaderDefinitions +
      "const AtmosphereParameters ATMOSPHERE = AtmosphereParameters(\n" +
          to_string(solar_irradiance, lambdas, 1.0) + ",\n" +
          std::to_string(sun_angular_radius) + ",\n" +
          std::to_string(bottom_radius / length_unit_in_meters) + ",\n" +
          std::to_string(top_radius / length_unit_in_meters) + ",\n" +
          density_profile(rayleigh_density) + ",\n" +
          to_string(rayleigh_scattering, lambdas, length_unit_in_meters) + ",\n" +
          density_profile(mie_density) + ",\n" +
          to_string(mie_scattering, lambdas, length_unit_in_meters) + ",\n" +
          to_string(mie_extinction, lambdas, length_unit_in_meters) + ",\n" +
          std::to_string(mie_phase_function_g) + ",\n" +
          density_profile(absorption_density) + ",\n" +
          to_string(absorption_extinction, lambdas, length_unit_in_meters) + ",\n" +
          to_string(ground_albedo, lambdas, 1.0) + ",\n" +
          std::to_string(cos(max_sun_zenith_angle)) + ");\n" +
      "const vec3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(" +
          std::to_string(sky_k_r) + "," +
          std::to_string(sky_k_g) + "," +
          std::to_string(sky_k_b) + ");\n" +
      "const vec3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(" +
          std::to_string(sun_k_r) + "," +
          std::to_string(sun_k_g) + "," +
          std::to_string(sun_k_b) + ");\n" +
      atmosphere::kFragmentShaderFunctions;
  };

  // Allocate the precomputed textures, but don't precompute them yet.
  transmittance_texture_ = NewTexture2d(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
  scattering_texture_ = NewTexture3d(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH,
      combine_scattering_textures || !rgb_format_supported_ ? GL_RGBA : GL_RGB,half_precision);
  if (combine_scattering_textures) {
    optional_single_mie_scattering_texture_ = 0;
  } else {
    optional_single_mie_scattering_texture_ = NewTexture3d(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH,
                                                           rgb_format_supported_ ? GL_RGB : GL_RGBA,half_precision);
  }
  irradiance_texture_ = NewTexture2d(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);

    // Create and compile the shader providing our API.
  std::string shader =
      glsl_header_factory_({kLambdaR, kLambdaG, kLambdaB}) +
      (precompute_illuminance ? "" : "#define RADIANCE_API_ENABLED\n") +
      atmosphere::kAtmosphereShader;
  const char* source = shader.c_str();
  atmosphere_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(atmosphere_shader_, 1, &source, NULL);
  glCompileShader(atmosphere_shader_);

  // std::ofstream of("/Users/huzhihao/Code/Playground/atmosphere.glsl");
  // of << source << "\n";
  // of.flush();
  // of.close();

  // Create a full screen quad vertex array and vertex buffer objects.
  glGenVertexArrays(1, &full_screen_quad_vao_);
  glBindVertexArray(full_screen_quad_vao_);
  glGenBuffers(1, &full_screen_quad_vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, full_screen_quad_vbo_);
  const GLfloat vertices[] = {
    -1.0, -1.0,
    +1.0, -1.0,
    -1.0, +1.0,
    +1.0, +1.0,
  };
  constexpr int kCoordsPerVertex = 2;
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  constexpr GLuint kAttribIndex = 0;
  glVertexAttribPointer(kAttribIndex, kCoordsPerVertex, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(kAttribIndex);
  glBindVertexArray(0);
}

GEngine::PrecomputedAtmosphereModel::~PrecomputedAtmosphereModel() {}

void GEngine::PrecomputedAtmosphereModel::Init(
    unsigned int num_scattering_orders) {
  GLuint delta_irradiance_texture = NewTexture2d(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
  GLuint delta_rayleigh_scattering_texture = NewTexture3d(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
                                                          SCATTERING_TEXTURE_DEPTH,
                                                          rgb_format_supported_ ? GL_RGB : GL_RGBA, half_precision_);
  GLuint delta_mie_scattering_texture = NewTexture3d(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
                                                     SCATTERING_TEXTURE_DEPTH,
                                                     rgb_format_supported_ ? GL_RGB : GL_RGBA, half_precision_);
  GLuint delta_scattering_density_texture = NewTexture3d(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,
                                                         SCATTERING_TEXTURE_DEPTH,
                                                         rgb_format_supported_ ? GL_RGB : GL_RGBA, half_precision_);

  GLuint delta_multiple_scattering_texture = delta_rayleigh_scattering_texture;

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  if (num_precomputed_wavelengths_ <= 3) {
    vec3 lambdas{kLambdaR, kLambdaG, kLambdaB};
    mat3 luminance_from_radiance{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    Precompute(fbo, delta_irradiance_texture, delta_rayleigh_scattering_texture,
        delta_mie_scattering_texture, delta_scattering_density_texture,
        delta_multiple_scattering_texture, lambdas, luminance_from_radiance,
        false /* blend */, num_scattering_orders);
  } else {
    constexpr double kLambdaMin = 360.0;
    constexpr double kLambdaMax = 830.0;
    int num_iterations = (num_precomputed_wavelengths_ + 2) / 3;
    double dlambda = (kLambdaMax - kLambdaMin) / (3 * num_iterations);
    for (int i = 0; i < num_iterations; ++i) {
      vec3 lambdas{
        kLambdaMin + (3 * i + 0.5) * dlambda,
        kLambdaMin + (3 * i + 1.5) * dlambda,
        kLambdaMin + (3 * i + 2.5) * dlambda
      };
      auto coeff = [dlambda](double lambda, int component) {
        // Note that we don't include MAX_LUMINOUS_EFFICACY here, to avoid
        // artefacts due to too large values when using half precision on GPU.
        // We add this term back in kAtmosphereShader, via
        // SKY_SPECTRAL_RADIANCE_TO_LUMINANCE (see also the comments in the
        // Model constructor).
        double x = CieColorMatchingFunctionTableValue(lambda, 1);
        double y = CieColorMatchingFunctionTableValue(lambda, 2);
        double z = CieColorMatchingFunctionTableValue(lambda, 3);
        return static_cast<float>((
            XYZ_TO_SRGB[component * 3] * x +
            XYZ_TO_SRGB[component * 3 + 1] * y +
            XYZ_TO_SRGB[component * 3 + 2] * z) * dlambda);
      };
      mat3 luminance_from_radiance{
        coeff(lambdas[0], 0), coeff(lambdas[1], 0), coeff(lambdas[2], 0),
        coeff(lambdas[0], 1), coeff(lambdas[1], 1), coeff(lambdas[2], 1),
        coeff(lambdas[0], 2), coeff(lambdas[1], 2), coeff(lambdas[2], 2)
      };
      Precompute(fbo, delta_irradiance_texture,
          delta_rayleigh_scattering_texture, delta_mie_scattering_texture,
          delta_scattering_density_texture, delta_multiple_scattering_texture,
          lambdas, luminance_from_radiance, i > 0 /* blend */,
          num_scattering_orders);
    }
    std::string header = glsl_header_factory_({kLambdaR, kLambdaG, kLambdaB});
    Program compute_transmittance(atmosphere::kVertexShader, header + atmosphere::kComputeTransmittanceShader);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, transmittance_texture_, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
    compute_transmittance.Use();
    DrawQuad({}, full_screen_quad_vao_);
  }

  // Delete the temporary resources allocated at the begining of this method.
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &delta_scattering_density_texture);
  glDeleteTextures(1, &delta_mie_scattering_texture);
  glDeleteTextures(1, &delta_rayleigh_scattering_texture);
  glDeleteTextures(1, &delta_irradiance_texture);
  assert(glGetError() == 0);
}

void GEngine::PrecomputedAtmosphereModel::SetProgramUniforms(
    GLuint program, GLuint transmittance_texture_unit,
    GLuint scattering_texture_unit, GLuint irradiance_texture_unit,
    GLuint single_mie_scattering_texture_unit) const {
  glActiveTexture(GL_TEXTURE0 + transmittance_texture_unit);
  glBindTexture(GL_TEXTURE_2D, transmittance_texture_);
  glUniform1i(glGetUniformLocation(program, "transmittance_texture"), transmittance_texture_unit);

  glActiveTexture(GL_TEXTURE0 + scattering_texture_unit);
  glBindTexture(GL_TEXTURE_3D, scattering_texture_);
  glUniform1i(glGetUniformLocation(program, "scattering_texture"), scattering_texture_unit);

  glActiveTexture(GL_TEXTURE0 + irradiance_texture_unit);
  glBindTexture(GL_TEXTURE_2D, irradiance_texture_);
  glUniform1i(glGetUniformLocation(program, "irradiance_texture"), irradiance_texture_unit);

  if (optional_single_mie_scattering_texture_ != 0) {
    glActiveTexture(GL_TEXTURE0 + single_mie_scattering_texture_unit);
    glBindTexture(GL_TEXTURE_3D, optional_single_mie_scattering_texture_);
    glUniform1i(glGetUniformLocation(program, "single_mie_scattering_texture"), single_mie_scattering_texture_unit);
  }
}

void GEngine::PrecomputedAtmosphereModel::ConvertSpectrumToLinearSrgb(
    const std::vector<double>& wavelengths,
    const std::vector<double>& spectrum,
    double* r, double* g, double* b) {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  const int dlambda = 1;
  for (int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
    double value = Interpolate(wavelengths, spectrum, lambda);
    x += CieColorMatchingFunctionTableValue(lambda, 1) * value;
    y += CieColorMatchingFunctionTableValue(lambda, 2) * value;
    z += CieColorMatchingFunctionTableValue(lambda, 3) * value;
  }
  *r = MAX_LUMINOUS_EFFICACY *
      (XYZ_TO_SRGB[0] * x + XYZ_TO_SRGB[1] * y + XYZ_TO_SRGB[2] * z) * dlambda;
  *g = MAX_LUMINOUS_EFFICACY *
      (XYZ_TO_SRGB[3] * x + XYZ_TO_SRGB[4] * y + XYZ_TO_SRGB[5] * z) * dlambda;
  *b = MAX_LUMINOUS_EFFICACY *
      (XYZ_TO_SRGB[6] * x + XYZ_TO_SRGB[7] * y + XYZ_TO_SRGB[8] * z) * dlambda;
}

void GEngine::PrecomputedAtmosphereModel::Precompute(
    GLuint fbo,
    GLuint delta_irradiance_texture,
    GLuint delta_rayleigh_scattering_texture,
    GLuint delta_mie_scattering_texture,
    GLuint delta_scattering_density_texture,
    GLuint delta_multiple_scattering_texture,
    const vec3& lambdas,
    const mat3& luminance_from_radiance,
    bool blend,
    unsigned int num_scattering_orders) {
  // The precomputations require specific GLSL programs, for each precomputation
  // step. We create and compile them here (they are automatically destroyed
  // when this method returns, via the Program destructor).
  std::string header = glsl_header_factory_(lambdas);
//  std::cout << atmosphere::kVertexShader << '\n';
//  std::cout << header << '\n';
  Program compute_transmittance(atmosphere::kVertexShader, header + atmosphere::kComputeTransmittanceShader);
  Program compute_direct_irradiance(atmosphere::kVertexShader, header + atmosphere::kComputeDirectIrradianceShader);
  Program compute_single_scattering(atmosphere::kVertexShader, atmosphere::kGeometryShader, header + atmosphere::kComputeSingleScatteringShader);
  Program compute_scattering_density(atmosphere::kVertexShader, atmosphere::kGeometryShader, header + atmosphere::kComputeScatteringDensityShader);
  Program compute_indirect_irradiance(atmosphere::kVertexShader, header + atmosphere::kComputeIndirectIrradianceShader);
  Program compute_multiple_scattering(atmosphere::kVertexShader, atmosphere::kGeometryShader, header + atmosphere::kComputeMultipleScatteringShader);

  const GLuint kDrawBuffers[4] = {
    GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3
  };
  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
  glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

  // Compute the transmittance, and store it in transmittance_texture_.
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, transmittance_texture_, 0);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glViewport(0, 0, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
  compute_transmittance.Use();
  DrawQuad({}, full_screen_quad_vao_);

  // Compute the direct irradiance, store it in delta_irradiance_texture and,
  // depending on 'blend', either initialize irradiance_texture_ with zeros or
  // leave it unchanged (we don't want the direct irradiance in
  // irradiance_texture_, but only the irradiance from the sky).
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_irradiance_texture, 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, irradiance_texture_, 0);
  glDrawBuffers(2, kDrawBuffers);
  glViewport(0, 0, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
  compute_direct_irradiance.Use();
  compute_direct_irradiance.BindTexture2d("transmittance_texture", transmittance_texture_, 0);
  DrawQuad({false, blend}, full_screen_quad_vao_);

  // Compute the rayleigh and mie single scattering, store them in
  // delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
  // either store them or accumulate them in scattering_texture_ and
  // optional_single_mie_scattering_texture_.
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_rayleigh_scattering_texture, 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, delta_mie_scattering_texture, 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, scattering_texture_, 0);
  if (optional_single_mie_scattering_texture_ != 0) {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, optional_single_mie_scattering_texture_, 0);
    glDrawBuffers(4, kDrawBuffers);
  } else {
    glDrawBuffers(3, kDrawBuffers);
  }
  glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
  compute_single_scattering.Use();
  compute_single_scattering.BindMat3("luminance_from_radiance", luminance_from_radiance);
  compute_single_scattering.BindTexture2d("transmittance_texture", transmittance_texture_, 0);
  for (unsigned int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
    compute_single_scattering.BindInt("layer", layer);
    DrawQuad({false, false, blend, blend}, full_screen_quad_vao_);
  }

  // Compute the 2nd, 3rd and 4th order of scattering, in sequence.
  for (unsigned int scattering_order = 2; scattering_order <= num_scattering_orders;++scattering_order) {
    // Compute the scattering density, and store it in
    // delta_scattering_density_texture.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_scattering_density_texture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
    compute_scattering_density.Use();
    compute_scattering_density.BindTexture2d("transmittance_texture", transmittance_texture_, 0);
    compute_scattering_density.BindTexture3d("single_rayleigh_scattering_texture", delta_rayleigh_scattering_texture, 1);
    compute_scattering_density.BindTexture3d("single_mie_scattering_texture", delta_mie_scattering_texture, 2);
    compute_scattering_density.BindTexture3d("multiple_scattering_texture", delta_multiple_scattering_texture, 3);
    compute_scattering_density.BindTexture2d("irradiance_texture", delta_irradiance_texture, 4);
    compute_scattering_density.BindInt("scattering_order", scattering_order);
    for (unsigned int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
      compute_scattering_density.BindInt("layer", layer);
      DrawQuad({}, full_screen_quad_vao_);
    }

    // Compute the indirect irradiance, store it in delta_irradiance_texture and
    // accumulate it in irradiance_texture_.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_irradiance_texture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, irradiance_texture_, 0);
    glDrawBuffers(2, kDrawBuffers);
    glViewport(0, 0, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
    compute_indirect_irradiance.Use();
    compute_indirect_irradiance.BindMat3("luminance_from_radiance", luminance_from_radiance);
    compute_indirect_irradiance.BindTexture3d("single_rayleigh_scattering_texture", delta_rayleigh_scattering_texture, 0);
    compute_indirect_irradiance.BindTexture3d("single_mie_scattering_texture", delta_mie_scattering_texture, 1);
    compute_indirect_irradiance.BindTexture3d("multiple_scattering_texture", delta_multiple_scattering_texture, 2);
    compute_indirect_irradiance.BindInt("scattering_order", scattering_order - 1);
    DrawQuad({false, true}, full_screen_quad_vao_);

    // Compute the multiple scattering, store it in
    // delta_multiple_scattering_texture, and accumulate it in
    // scattering_texture_.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_multiple_scattering_texture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, scattering_texture_, 0);
    glDrawBuffers(2, kDrawBuffers);
    glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
    compute_multiple_scattering.Use();
    compute_multiple_scattering.BindMat3("luminance_from_radiance", luminance_from_radiance);
    compute_multiple_scattering.BindTexture2d("transmittance_texture", transmittance_texture_, 0);
    compute_multiple_scattering.BindTexture3d("scattering_density_texture", delta_scattering_density_texture, 1);
    for (unsigned int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
      compute_multiple_scattering.BindInt("layer", layer);
      DrawQuad({false, true}, full_screen_quad_vao_);
    }
  }
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
}
