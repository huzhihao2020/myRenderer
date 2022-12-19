#pragma once
#include "GEngine/material.h"
#include "GEngine/shader.h"
#include "GEngine/texture.h"
#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <vector>

namespace GEngine {

struct SBoneInfo {
  int id;
  glm::mat4 inverse_bind_transform;
};

class CMesh {
public:
  // for Structure of Arrays
  enum BUFFER_TYPE : uint8_t {
    INDEX_BUFFER = 0u,
    POSITION,
    NORMAL,
    TEXCOORD,
    TANGENT,
    BONE_ID,
    WEIGHTS,
    // MVP_MAT & WORLD_MAT is only for instancing
    // MVP_MAT,
    // WORLD_MAT,
    // NUM_BUFFERS is the numbers of buffers, not a buffer type
    NUM_BUFFERS,
  };

  // MeshEntry stores the sub-components of the model
  // For example: a charater model can be composed of head, body, weapon...
  // this allows you apply some tranform on the component without affceting the others
  struct SMeshEntry {
    SMeshEntry() {
      num_indices_ = 0;
      base_vertex_ = 0;
      base_index_ = 0;
      material_index_ = -1;
    }

    unsigned int num_indices_;
    unsigned int base_vertex_;
    unsigned int base_index_;
    int material_index_;
  };

  CMesh();
  ~CMesh();
  
  bool LoadMesh(const std::string &filename);
  void Render(std::shared_ptr<GEngine::Shader> shader);
  void Clear();

  // bone info getter
  std::map<std::string, std::shared_ptr<SBoneInfo>>& GetBoneInfoMap() { return bone_info_; }
  int& GetBoneCount() { return bone_counter_; }

  // mesh data (new)
  std::vector<SMeshEntry> meshes_;
  std::vector<std::shared_ptr<CMaterial>> materials_;
  std::vector<std::shared_ptr<CTexture>> loaded_textures_;

  unsigned int VAO_, EBO_;

private:
  unsigned int buffers_[NUM_BUFFERS] = {0};
  
  bool InitFromScene(const aiScene* scene, const std::string &filename);

  std::tuple<unsigned int, unsigned int>
  CountTotalVerticesAndIndices(const aiScene *scene);

  bool ParseMaterials(const aiScene *scene, const std::string &filename);

  bool LoadMaterialTexture(const aiMaterial *mat,
                           std::string &dir,
                           int index,
                           aiTextureType type,
                           std::shared_ptr<CTexture>& texture);

  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> texcoords_;
  std::vector<glm::vec3> tangents_;
  std::vector<glm::ivec4> bone_ids_;
  std::vector<glm::vec4>  weights_;
  std::vector<unsigned int> indices_;

  int num_faces_ = 0;

  // bool has_animation_ = false;
  int bone_counter_ = 0;
  std::map<std::string, std::shared_ptr<SBoneInfo>> bone_info_;
};

} // namespace GEngine