#include "GEngine/renderpass/ltc_demo_pass.h"
#include "GEngine/texture.h"
#include "GEngine/renderpass/ltc_matrix.h"
#include "GEngine/render_system.h"

#include <memory>
#include <string>

GEngine::LTCDemoPass::LTCDemoPass(const std::string& name, int order) : CRenderPass(name, order){}

GEngine::LTCDemoPass::~LTCDemoPass(){}

  void GEngine::LTCDemoPass::Init(){

  const GLfloat psize = 10.0f;
  float plane_vertices[] = {
    // pos                // normal         // texcoord
    -psize, 0.0f, -psize, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    -psize, 0.0f,  psize, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     psize, 0.0f,  psize, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    -psize, 0.0f, -psize, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
     psize, 0.0f,  psize, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
     psize, 0.0f, -psize, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f
  };
  float areaLight_vertices[] = {
    -8.0f, 2.4f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 0 1 5 4
    -8.0f, 2.4f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -8.0f, 0.4f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    -8.0f, 2.4f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -8.0f, 0.4f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    -8.0f, 0.4f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f
  };

  // load LTC1 & LTC2
  ltc1_ = LoadLTCTexture(GEngine::LTC1);
  ltc2_ = LoadLTCTexture(GEngine::LTC2);
  std::string texture_path("../../assets/textures/marble.jpg");
  plane_texture_ = std::make_shared<CTexture>(texture_path);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ltc1_);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, ltc2_);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, plane_texture_->id_);

  std::string v1_path("../../GEngine/src/GEngine/renderpass/ltc_demo_vert.glsl");
  std::string f1_path("../../GEngine/src/GEngine/renderpass/ltc_demo_frag.glsl");
  std::string v2_path("../../GEngine/src/GEngine/renderpass/ltc_demo_plane_vert.glsl");
  std::string f2_path("../../GEngine/src/GEngine/renderpass/ltc_demo_plane_frag.glsl");
  shader_ = std::make_shared<Shader>(v1_path, f1_path);
  light_plane_shader_ = std::make_shared<Shader>(v2_path, f2_path);

  shader_->Use();
  shader_->SetVec3("areaLight.points[0]", glm::vec3(-8.0f, 2.4f, -1.0f));
  shader_->SetVec3("areaLight.points[1]", glm::vec3(-8.0f, 2.4f,  1.0f));
  shader_->SetVec3("areaLight.points[2]", glm::vec3(-8.0f, 0.4f,  1.0f));
  shader_->SetVec3("areaLight.points[3]", glm::vec3(-8.0f, 0.4f, -1.0f));
  shader_->SetVec3("areaLight.color", light_color_);
  shader_->SetInt("LTC1", 0);
  shader_->SetInt("LTC2", 1);
  shader_->SetInt("material.diffuse", 2);
	IncrementRoughness(0.0f);
	IncrementLightIntensity(0.0f);
	SwitchTwoSided(false);
  glUseProgram(0);

  light_plane_shader_->Use();
	{
		glm::mat4 model(1.0f);
		light_plane_shader_->SetMat4("model", model);
	}
	light_plane_shader_->SetVec3("lightColor", light_color_);
	glUseProgram(0);

	// 3D OBJECTS
	ConfigureMockupData(plane_vertices, areaLight_vertices);
	AreaLightTranslate_ = glm::vec3(0.0f, 0.0f, 0.0f);

  glEnable(GL_DEPTH_TEST);
}

unsigned GEngine::LTCDemoPass::LoadLTCTexture(const float matrixTable[]) {
  unsigned int texture = 0;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_FLOAT, matrixTable);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  return texture;
}

void GEngine::LTCDemoPass::Tick(){
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shader_->Use();
  glm::mat4 model(1.0f);
  glm::mat3 normalMatrix = glm::mat3(model);
  shader_->SetMat4("model", model);
  shader_->SetMat3("normalMatrix", normalMatrix);
  glm::mat4 view = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetViewMatrix();
  glm::mat4 projection = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetProjectionMatrix();
  shader_->SetMat4("view", view);
  shader_->SetMat4("projection", projection);
  glm::vec3 camera_pos = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetPosition();
  shader_->SetVec3("viewPosition", camera_pos);
  shader_->SetVec3("areaLightTranslate", AreaLightTranslate_);

  shader_->SetVec3("areaLight.points[0]", glm::vec3(-8.0f, 2.4f, -1.0f));
  shader_->SetVec3("areaLight.points[1]", glm::vec3(-8.0f, 2.4f,  1.0f));
  shader_->SetVec3("areaLight.points[2]", glm::vec3(-8.0f, 0.4f,  1.0f));
  shader_->SetVec3("areaLight.points[3]", glm::vec3(-8.0f, 0.4f, -1.0f));
  auto color_from_ui = CSingleton<CRenderSystem>()->GetOrCreateMainUI()->GetColorPickerAreaLight();
  light_color_ = glm::vec3(color_from_ui[0], color_from_ui[1], color_from_ui[2]);
  shader_->SetVec3("areaLight.color", light_color_);
  glActiveTexture(GL_TEXTURE0); 
  glBindTexture(GL_TEXTURE_2D, ltc1_);
  shader_->SetInt("LTC1", 0);
  glActiveTexture(GL_TEXTURE1); 
  glBindTexture(GL_TEXTURE_2D, ltc2_);
  shader_->SetInt("LTC2", 1);
  glActiveTexture(GL_TEXTURE2); 
  glBindTexture(GL_TEXTURE_2D, plane_texture_->id_);
  shader_->SetInt("material.diffuse", 2);
//  IncrementRoughness(0.0f);
//	IncrementLightIntensity(0.0f);
//	SwitchTwoSided(false);
  RenderPlane();
  glUseProgram(0);

  light_plane_shader_->Use();
  model = glm::translate(model, AreaLightTranslate_);
  light_plane_shader_->SetMat4("model", model);
  light_plane_shader_->SetMat4("view", view);
  light_plane_shader_->SetMat4("projection", projection);
  light_plane_shader_->SetVec3("lightColor", light_color_);
  RenderAreaLight();
  glUseProgram(0);
}

void GEngine::LTCDemoPass::RenderPlane()
{
	glBindVertexArray(planeVAO_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void GEngine::LTCDemoPass::RenderAreaLight()
{
	glBindVertexArray(areaLightVAO_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void GEngine::LTCDemoPass::IncrementRoughness(float step)
{
	static glm::vec3 color = glm::vec3(0.5f, 0.5f, 0.5f);
	static float roughness = 0.5f;
	roughness += step;
	roughness = glm::clamp(roughness, 0.0f, 1.0f);
	//std::cout << "roughness: " << roughness << '\n';
	shader_->Use();
	shader_->SetVec4("material.albedoRoughness", glm::vec4(color, roughness));
	glUseProgram(0);
}

void GEngine::LTCDemoPass::IncrementLightIntensity(float step)
{
	static float intensity = 4.0f;
	intensity += step;
	intensity = glm::clamp(intensity, 0.0f, 10.0f);
	//std::cout << "intensity: " << intensity << '\n';
	shader_->Use();
	shader_->SetFloat("areaLight.intensity", intensity);
	glUseProgram(0);
}

void GEngine::LTCDemoPass::SwitchTwoSided(bool doSwitch)
{
	static bool twoSided = true;
	if (doSwitch) twoSided = !twoSided;
	//std::cout << "twoSided: " << std::boolalpha << twoSided << '\n';
	shader_->Use();
	shader_->SetFloat("areaLight.twoSided", twoSided);
	glUseProgram(0);
}

void GEngine::LTCDemoPass::ConfigureMockupData(float plane_vertices[], float areaLight_vertices[]) {
      // PLANE
    glGenVertexArrays(1, &planeVAO_);
    glGenBuffers(1, &planeVBO_);

    glBindVertexArray(planeVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO_);
    glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(plane_vertices[0]), &plane_vertices[0], GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // AREA LIGHT
    glGenVertexArrays(1, &areaLightVAO_);
    glBindVertexArray(areaLightVAO_);

    glGenBuffers(1, &areaLightVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, areaLightVBO_);
    glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(areaLight_vertices[0]), &areaLight_vertices[0], GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    glBindVertexArray(0);
}
