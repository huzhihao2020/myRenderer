#pragma Once

#include "GEngine/render_pass.h"
#include "GEngine/shader.h"

#include <string>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace GEngine {

constexpr int TRANSMITTANCE_TEXTURE_WIDTH = 256;
constexpr int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

constexpr int SCATTERING_TEXTURE_R_SIZE = 32;
constexpr int SCATTERING_TEXTURE_MU_SIZE = 128;
constexpr int SCATTERING_TEXTURE_MU_S_SIZE = 32;
constexpr int SCATTERING_TEXTURE_NU_SIZE = 8;

constexpr int SCATTERING_TEXTURE_WIDTH = SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE;
constexpr int SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;
constexpr int SCATTERING_TEXTURE_DEPTH = SCATTERING_TEXTURE_R_SIZE;

constexpr int IRRADIANCE_TEXTURE_WIDTH = 64;
constexpr int IRRADIANCE_TEXTURE_HEIGHT = 16;

constexpr double MAX_LUMINOUS_EFFICACY = 683.0;

constexpr double CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[380] = {
  360, 0.000129900000, 0.000003917000, 0.000606100000,
  365, 0.000232100000, 0.000006965000, 0.001086000000,
  370, 0.000414900000, 0.000012390000, 0.001946000000,
  375, 0.000741600000, 0.000022020000, 0.003486000000,
  380, 0.001368000000, 0.000039000000, 0.006450001000,
  385, 0.002236000000, 0.000064000000, 0.010549990000,
  390, 0.004243000000, 0.000120000000, 0.020050010000,
  395, 0.007650000000, 0.000217000000, 0.036210000000,
  400, 0.014310000000, 0.000396000000, 0.067850010000,
  405, 0.023190000000, 0.000640000000, 0.110200000000,
  410, 0.043510000000, 0.001210000000, 0.207400000000,
  415, 0.077630000000, 0.002180000000, 0.371300000000,
  420, 0.134380000000, 0.004000000000, 0.645600000000,
  425, 0.214770000000, 0.007300000000, 1.039050100000,
  430, 0.283900000000, 0.011600000000, 1.385600000000,
  435, 0.328500000000, 0.016840000000, 1.622960000000,
  440, 0.348280000000, 0.023000000000, 1.747060000000,
  445, 0.348060000000, 0.029800000000, 1.782600000000,
  450, 0.336200000000, 0.038000000000, 1.772110000000,
  455, 0.318700000000, 0.048000000000, 1.744100000000,
  460, 0.290800000000, 0.060000000000, 1.669200000000,
  465, 0.251100000000, 0.073900000000, 1.528100000000,
  470, 0.195360000000, 0.090980000000, 1.287640000000,
  475, 0.142100000000, 0.112600000000, 1.041900000000,
  480, 0.095640000000, 0.139020000000, 0.812950100000,
  485, 0.057950010000, 0.169300000000, 0.616200000000,
  490, 0.032010000000, 0.208020000000, 0.465180000000,
  495, 0.014700000000, 0.258600000000, 0.353300000000,
  500, 0.004900000000, 0.323000000000, 0.272000000000,
  505, 0.002400000000, 0.407300000000, 0.212300000000,
  510, 0.009300000000, 0.503000000000, 0.158200000000,
  515, 0.029100000000, 0.608200000000, 0.111700000000,
  520, 0.063270000000, 0.710000000000, 0.078249990000,
  525, 0.109600000000, 0.793200000000, 0.057250010000,
  530, 0.165500000000, 0.862000000000, 0.042160000000,
  535, 0.225749900000, 0.914850100000, 0.029840000000,
  540, 0.290400000000, 0.954000000000, 0.020300000000,
  545, 0.359700000000, 0.980300000000, 0.013400000000,
  550, 0.433449900000, 0.994950100000, 0.008749999000,
  555, 0.512050100000, 1.000000000000, 0.005749999000,
  560, 0.594500000000, 0.995000000000, 0.003900000000,
  565, 0.678400000000, 0.978600000000, 0.002749999000,
  570, 0.762100000000, 0.952000000000, 0.002100000000,
  575, 0.842500000000, 0.915400000000, 0.001800000000,
  580, 0.916300000000, 0.870000000000, 0.001650001000,
  585, 0.978600000000, 0.816300000000, 0.001400000000,
  590, 1.026300000000, 0.757000000000, 0.001100000000,
  595, 1.056700000000, 0.694900000000, 0.001000000000,
  600, 1.062200000000, 0.631000000000, 0.000800000000,
  605, 1.045600000000, 0.566800000000, 0.000600000000,
  610, 1.002600000000, 0.503000000000, 0.000340000000,
  615, 0.938400000000, 0.441200000000, 0.000240000000,
  620, 0.854449900000, 0.381000000000, 0.000190000000,
  625, 0.751400000000, 0.321000000000, 0.000100000000,
  630, 0.642400000000, 0.265000000000, 0.000049999990,
  635, 0.541900000000, 0.217000000000, 0.000030000000,
  640, 0.447900000000, 0.175000000000, 0.000020000000,
  645, 0.360800000000, 0.138200000000, 0.000010000000,
  650, 0.283500000000, 0.107000000000, 0.000000000000,
  655, 0.218700000000, 0.081600000000, 0.000000000000,
  660, 0.164900000000, 0.061000000000, 0.000000000000,
  665, 0.121200000000, 0.044580000000, 0.000000000000,
  670, 0.087400000000, 0.032000000000, 0.000000000000,
  675, 0.063600000000, 0.023200000000, 0.000000000000,
  680, 0.046770000000, 0.017000000000, 0.000000000000,
  685, 0.032900000000, 0.011920000000, 0.000000000000,
  690, 0.022700000000, 0.008210000000, 0.000000000000,
  695, 0.015840000000, 0.005723000000, 0.000000000000,
  700, 0.011359160000, 0.004102000000, 0.000000000000,
  705, 0.008110916000, 0.002929000000, 0.000000000000,
  710, 0.005790346000, 0.002091000000, 0.000000000000,
  715, 0.004109457000, 0.001484000000, 0.000000000000,
  720, 0.002899327000, 0.001047000000, 0.000000000000,
  725, 0.002049190000, 0.000740000000, 0.000000000000,
  730, 0.001439971000, 0.000520000000, 0.000000000000,
  735, 0.000999949300, 0.000361100000, 0.000000000000,
  740, 0.000690078600, 0.000249200000, 0.000000000000,
  745, 0.000476021300, 0.000171900000, 0.000000000000,
  750, 0.000332301100, 0.000120000000, 0.000000000000,
  755, 0.000234826100, 0.000084800000, 0.000000000000,
  760, 0.000166150500, 0.000060000000, 0.000000000000,
  765, 0.000117413000, 0.000042400000, 0.000000000000,
  770, 0.000083075270, 0.000030000000, 0.000000000000,
  775, 0.000058706520, 0.000021200000, 0.000000000000,
  780, 0.000041509940, 0.000014990000, 0.000000000000,
  785, 0.000029353260, 0.000010600000, 0.000000000000,
  790, 0.000020673830, 0.000007465700, 0.000000000000,
  795, 0.000014559770, 0.000005257800, 0.000000000000,
  800, 0.000010253980, 0.000003702900, 0.000000000000,
  805, 0.000007221456, 0.000002607800, 0.000000000000,
  810, 0.000005085868, 0.000001836600, 0.000000000000,
  815, 0.000003581652, 0.000001293400, 0.000000000000,
  820, 0.000002522525, 0.000000910930, 0.000000000000,
  825, 0.000001776509, 0.000000641530, 0.000000000000,
  830, 0.000001251141, 0.000000451810, 0.000000000000,
};

constexpr double XYZ_TO_SRGB[9] = {
  +3.2406, -1.5372, -0.4986,
  -0.9689, +1.8758, +0.0415,
  +0.0557, -0.2040, +1.0570
};

/**
 * @brief DensityProfileLayer
 * 
 */
class DensityProfileLayer {
 public:
  DensityProfileLayer() : DensityProfileLayer(0.0, 0.0, 0.0, 0.0, 0.0) {}
  DensityProfileLayer(double width, double exp_term, double exp_scale, double linear_term, double constant_term)
      : width(width), exp_term(exp_term), exp_scale(exp_scale), linear_term(linear_term), constant_term(constant_term) {
  }

  double width;
  double exp_term;
  double exp_scale;
  double linear_term;
  double constant_term;
};

/**
 * @brief PrecomputedAtmosphereModel
 * 
 */
class PrecomputedAtmosphereModel {
  public:

  PrecomputedAtmosphereModel(
    const std::vector<double>& wavelengths,
    const std::vector<double>& solar_irradiance,
    double sun_angular_radius,
    double bottom_radius,
    double top_radius,
    const std::vector<DensityProfileLayer>& rayleigh_density,
    const std::vector<double>& rayleigh_scattering,
    const std::vector<DensityProfileLayer>& mie_density,
    const std::vector<double>& mie_scattering,
    const std::vector<double>& mie_extinction,
    double mie_phase_function_g,
    const std::vector<DensityProfileLayer>& absorption_density,
    const std::vector<double>& absorption_extinction,
    const std::vector<double>& ground_albedo,
    double max_sun_zenith_angle,
    double length_unit_in_meters,
    unsigned int num_precomputed_wavelengths,
    bool combine_scattering_textures,
    bool half_precision);

    ~PrecomputedAtmosphereModel();

    void Init(unsigned int num_scattering_orders = 4);

    GLuint shader() const { return atmosphere_shader_; }

    void SetProgramUniforms(
        GLuint program, GLuint transmittance_texture_unit,
        GLuint scattering_texture_unit, GLuint irradiance_texture_unit,
        GLuint optional_single_mie_scattering_texture_unit = 0) const;

    static void ConvertSpectrumToLinearSrgb(const std::vector<double> &wavelengths,
                                            const std::vector<double> &spectrum, double *r,
                                            double *g, double *b);

    static constexpr int kLambdaMin = 360;
    static constexpr int kLambdaMax = 830;

    static constexpr double kLambdaR = 680.0;
    static constexpr double kLambdaG = 550.0;
    static constexpr double kLambdaB = 440.0;

    GLuint transmittance_texture_;
    GLuint scattering_texture_;
    GLuint optional_single_mie_scattering_texture_;
    GLuint irradiance_texture_;
    
  private:
    typedef std::array<double, 3> vec3;
    typedef std::array<float, 9> mat3;

    void Precompute(GLuint fbo, GLuint delta_irradiance_texture,
                    GLuint delta_rayleigh_scattering_texture,
                    GLuint delta_mie_scattering_texture,
                    GLuint delta_scattering_density_texture,
                    GLuint delta_multiple_scattering_texture,
                    const vec3 &lambdas, const mat3 &luminance_from_radiance,
                    bool blend, unsigned int num_scattering_orders);

    unsigned int num_precomputed_wavelengths_;
    bool half_precision_;
    bool rgb_format_supported_;

    std::function<std::string(const vec3 &)> glsl_header_factory_;

    GLuint atmosphere_shader_;
    GLuint full_screen_quad_vao_;
    GLuint full_screen_quad_vbo_;
};

/**
 * @brief PrecomputedAtmospherePass
 * 
 */
class PrecomputedAtmospherePass : public CRenderPass {
public:
  PrecomputedAtmospherePass(const std::string &name, int order);
  virtual ~PrecomputedAtmospherePass();

  virtual void Init() override;
  virtual void Tick() override;

  std::unique_ptr<PrecomputedAtmosphereModel> precomputed_atmosphere_model_;

private:
  enum Luminance {
    NONE,
    APPROXIMATE,
    PRECOMPUTED
  };

  constexpr static double kPi = 3.1415926;
  constexpr static double kSunAngularRadius = 0.00935 / 2.0;
  constexpr static double kSunSolidAngle = kPi * kSunAngularRadius * kSunAngularRadius;
  constexpr static double kLengthUnitInMeters = 1000.0;

  bool use_constant_solar_spectrum_ = false;
  bool use_ozone_ = true;
  bool use_combined_textures_ = false; // combine mie scatter texture to the alpha of scattering_texture_ texture
  bool use_half_precision_ = false;
  Luminance use_luminance_ = Luminance::NONE;
  bool do_white_balance_ = false;

  double view_distance_meters_ = 9000.0f;
  double view_zenith_angle_radians_ = 1.47f;
  double view_azimuth_angle_radians_ = -0.1f;
  double sun_zenith_angle_radians_ = 1.3f;
  double sun_azimuth_angle_radians_ = 2.9f;
  double exposure_ = 10.0f;

  GLuint transmittance_texture_;
  GLuint scattering_texture_;
  GLuint optional_single_mie_scattering_texture_;
  GLuint irradiance_texture_;

  std::unique_ptr<PrecomputedAtmosphereModel> model_;
  GLuint vertex_shader_;
  GLuint fragment_shader_;
  std::shared_ptr<Shader> program_;
  GLuint full_screen_quad_vao_;
  GLuint full_screen_quad_vbo_;

  // display texture on screen
  unsigned screenQuadVAO_;
  unsigned screenQuadVBO_;
};

} // namespace GEngine