#pragma once

#include "GEngine/render_pass.h"
#include "GEngine/renderpass/ltc_matrix.h" // for lut
#include "GEngine/shader.h"
#include <memory>

namespace GEngine {

  struct VertexAL {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
  };

  class LTCDemoPass : public CRenderPass {
    public:
    LTCDemoPass(const std::string &name, int order);
    virtual ~LTCDemoPass();

    virtual void Init() override;
    virtual void Tick() override;

    private:
    unsigned LoadLTCTexture(const float matrixTable[]);

    void RenderPlane();
    void RenderAreaLight();

    void IncrementRoughness(float step);
    void IncrementLightIntensity(float step);
    void SwitchTwoSided(bool doSwitch);

    void ConfigureMockupData(float plane_vertices[], float areaLight_vertices[]);

    std::shared_ptr<Shader> light_plane_shader_;
    glm::vec3 light_color_ = glm::vec3(0.7f, 0.7f, 0.7f);
    glm::vec3 AreaLightTranslate_ = glm::vec3(0.0f, 0.0f, 0.0f);

    GLuint planeVBO_, planeVAO_;
    GLuint areaLightVBO_, areaLightVAO_;

    unsigned int ltc1_;
    unsigned int ltc2_;
    std::shared_ptr<CTexture> plane_texture_;
  };
}