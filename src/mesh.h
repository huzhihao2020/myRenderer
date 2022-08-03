#pragma once
#include "shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

namespace GEngine {

const int MAX_BONE_INFLUENCE = 4;

struct SVertex {
  // struct 中的变量是连续存储的
  glm::vec3 position_;
  glm::vec3 normal_;
  glm::vec2 tex_coords_;
  // tangent space
  glm::vec3 tangent_;
  glm::vec3 bitangent_;
  // bone indexes which will influence this vertex & weights from each bone
  int bone_id_[MAX_BONE_INFLUENCE];
  float weights_[MAX_BONE_INFLUENCE];
};

struct STexture {
  unsigned int id_;
  std::string type_;
  std::string path_;
};

class CMesh {
public:
  CMesh(std::vector<SVertex> vertices,
        std::vector<unsigned int> indices,
        std::vector<STexture> textures);
  ~CMesh();
  void Draw(std::shared_ptr<CShader> shader);
  // Mesh Data
  std::vector<SVertex> vertices_;
  std::vector<unsigned int> indices_;
  std::vector<STexture> textures_;

  unsigned int VAO;

private:
  // rendering data
  unsigned int VBO, EBO;

  void SetupMesh();
};

} // namespace GEngine