#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mesh.h"
#include "shader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string>
#include <vector>

namespace GEngine {
class CModel {
public:
  // constructor, expects a filepath to a 3D model.
  CModel();
  CModel(std::string const &path, bool gamma = false);
  ~CModel();

  // draws the model, and thus all its meshes
  void Draw(std::shared_ptr<GEngine::CShader> shader);

private:
  std::vector<GEngine::STexture>  textures_loaded_; // stores all the textures loaded so far
  std::vector<GEngine::CMesh>     meshes_;
  const aiScene                   *scene_;        
  std::string                     directory_;
  bool                            gamma_correction_;
  // loads a model with supported ASSIMP extensions from file and stores the
  // resulting meshes in the meshes vector.
  void LoadModel(std::string const &path);

  // processes a node in a recursive fashion. Processes each individual mesh
  // located at the node and repeats this process on its children nodes (if
  // any).
  void ProcessNode(aiNode *node, const aiScene *scene);
  CMesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

  // checks all material textures of a given type and loads the textures if
  // they're not loaded yet. the required info is returned as a Texture struct.
  std::vector<STexture> LoadMaterialTextures(aiMaterial *mat,
                                             aiTextureType type,
                                             std::string type_name);
};

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
} // namespace GEngine
