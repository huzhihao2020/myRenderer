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
class CMesh {
public:
  // for Structure of Arrays
  enum BUFFER_TYPE {
    INDEX_BUFFER = 0u,
    POSITION     = 1u,
    NORMAL       = 2u,
    TEXCOORD     = 3u,
    // MVP_MAT & WORLD_MAT is only for instancing
    MVP_MAT      = 4u,
    WORLD_MAT    = 5u,
    // NUM_BUFFERS is the numbers of buffers, not a buffer type
    NUM_BUFFERS  = 6u, 
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
  
  bool LoadMesh(std::string &filename);
  void Render(std::shared_ptr<GEngine::CShader> shader);
  void Clear();

  // mesh data (new)
  std::vector<SMeshEntry> meshes_;
  std::vector<std::shared_ptr<CMaterial>> materials_;
  std::vector<std::shared_ptr<CTexture>> loaded_textures_;

  unsigned int VAO_, EBO_;

private:
  unsigned int buffers_[NUM_BUFFERS] = {0};
  void SetupMesh();
  
  bool InitFromScene(const aiScene* scene, std::string &filename);

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
  std::vector<unsigned int> indices_;
};

} // namespace GEngine