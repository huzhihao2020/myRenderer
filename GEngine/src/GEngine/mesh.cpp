#include "GEngine/mesh.h"
#include "GEngine/log.h"
#include "GEngine/material.h"
#include "GEngine/shader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include <tuple>

#define POISITION_LOCATION  0
#define NORMAL_LOCATION     1
#define TEX_COORD_LOCATION  2

GEngine::CMesh::CMesh() {}

GEngine::CMesh::~CMesh() {}

bool GEngine::CMesh::LoadMesh(std::string &filename) {
  bool success = false;

  // release previous loaded mesh
  Clear();

  // create VAO
  glGenVertexArrays(1, &VAO_);
  glBindVertexArray(VAO_);
  // Create buffers for vertex attributes
  glGenBuffers(NUM_BUFFERS, buffers_);
  // EBO
  glGenBuffers(1, &EBO_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int),&indices_[0], GL_STATIC_DRAW);

  Assimp::Importer importer;
  auto flags = aiProcess_Triangulate | 
               aiProcess_GenSmoothNormals |
               aiProcess_FlipUVs |
               aiProcess_CalcTangentSpace|
               aiProcess_JoinIdenticalVertices;
  const aiScene* ai_scene = importer.ReadFile(filename.c_str(), flags);
  if(ai_scene != nullptr) {
    success = InitFromScene(ai_scene, filename);
  }
  else {
    GE_ERROR("Failed parsing scene in {0}: {1}", filename, importer.GetErrorString());
  }

  glBindVertexArray(0);
  return success;
}

bool GEngine::CMesh::InitFromScene(const aiScene* scene, std::string &filename) {
  meshes_.resize(scene->mNumMeshes);
  materials_.resize(scene->mNumMaterials);

  // load the aiMesh datas from aiScene into our buffers
  auto [total_vertices, total_indices] = CountTotalVerticesAndIndices(scene);
  for(int i=0; i<materials_.size(); i++) {
    materials_[i] = std::make_shared<CMaterial>();
  }

  // reserve space for vertex data;
  normals_.reserve(total_vertices);
  positions_.reserve(total_vertices);
  texcoords_.reserve(total_vertices);
  indices_.reserve(total_indices);

  // traverse all meshes and init them
  for(int i=0; i<meshes_.size(); i++) {
    const aiVector3D zero3f(0.0f, 0.0f, 0.0f);
    const aiVector3D default_normal(0.0f, 1.0f, 0.0f);
    auto ai_mesh = scene->mMeshes[i];
    for(int jj=0; jj<ai_mesh->mNumVertices; jj++) {
      const auto& a_pos = ai_mesh->mVertices[jj];
      const auto& a_normal = ai_mesh->HasNormals() ? ai_mesh->mNormals[jj] : default_normal;
      const auto& a_texcoord = ai_mesh->HasTextureCoords(0) ? ai_mesh->mTextureCoords[0][jj] : zero3f;

      positions_.push_back(glm::vec3(a_pos.x, a_pos.y, a_pos.z));
      normals_.push_back(glm::vec3(a_normal.x, a_normal.y, a_normal.z));
      texcoords_.push_back(glm::vec2(a_texcoord.x, a_texcoord.y));
    }
    for(int kk=0; kk<ai_mesh->mNumFaces; kk++) {
      const auto& face = ai_mesh->mFaces[kk];
      if(face.mNumIndices != 3) {
        GE_ERROR("face indices number is not 3!");
      }
      indices_.push_back(face.mIndices[0]);
      indices_.push_back(face.mIndices[1]);
      indices_.push_back(face.mIndices[2]);
    }
  }

  // Parse Materials
  if(!ParseMaterials(scene, filename)){
    return false;
  }

  // Populate Buffers
  glBindBuffer(GL_ARRAY_BUFFER, buffers_[POSITION]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions_[0]) * positions_.size(), &positions_[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(POISITION_LOCATION);
  glVertexAttribPointer(POISITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, buffers_[NORMAL]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normals_[0]) * normals_.size(), &normals_[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(NORMAL_LOCATION);
  glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, buffers_[TEXCOORD]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords_[0]) * texcoords_.size(), &texcoords_[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(TEX_COORD_LOCATION);
  glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_[INDEX_BUFFER]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_[0]) * indices_.size(), &indices_[0], GL_STATIC_DRAW);

  return glGetError() == GL_NO_ERROR;
}

std::tuple<unsigned int, unsigned int>
GEngine::CMesh::CountTotalVerticesAndIndices(const aiScene *scene) {
  unsigned int num_vertices = 0;
  unsigned int num_indices = 0;

  for (int i = 0; i < meshes_.size(); i++) {
    meshes_[i].num_indices_ = scene->mMeshes[i]->mNumFaces * 3;
    meshes_[i].base_vertex_ = num_vertices;
    meshes_[i].base_index_ = num_indices;
    meshes_[i].material_index_ = scene->mMeshes[i]->mMaterialIndex;

    num_vertices += scene->mMeshes[i]->mNumVertices;
    num_indices += meshes_[i].num_indices_;
  }
  return std::make_tuple(num_vertices, num_indices);
}

bool GEngine::CMesh::ParseMaterials(const aiScene *scene, const std::string& filename) {
  auto slash_pos = filename.find_last_of('/');
  std::string filedir = slash_pos == std::string::npos ? "."
                        : slash_pos == 0               ? "/"
                                         : filename.substr(0, slash_pos);
  GE_INFO("Materials num - {0}", scene->mNumMaterials);

  // load textures
  for (int i = 0; i < scene->mNumMaterials; i++) {
    const aiMaterial *p_material = scene->mMaterials[i];
    // diffuse texture
    LoadMaterialTexture(p_material, filedir, i, aiTextureType_DIFFUSE, materials_[i]->diffuse_texture_);
    LoadMaterialTexture(p_material, filedir, i, aiTextureType_HEIGHT, materials_[i]->normal_texture_);
    LoadMaterialTexture(p_material, filedir, i, aiTextureType_BASE_COLOR, materials_[i]->basecolor_texture_);
    LoadMaterialTexture(p_material, filedir, i, aiTextureType_DIFFUSE_ROUGHNESS, materials_[i]->roughness_texture_);
    LoadMaterialTexture(p_material, filedir, i, aiTextureType_METALNESS, materials_[i]->mettalic_texture_);
    LoadMaterialTexture(p_material, filedir, i, aiTextureType_AMBIENT_OCCLUSION, materials_[i]->ao_texture_);
    LoadMaterialTexture(p_material, filedir, i, aiTextureType_EMISSION_COLOR, materials_[i]->emissive_texture_);
  }
  return true;
}

bool GEngine::CMesh::LoadMaterialTexture(const aiMaterial *material,
                                         std::string &filedir,
                                         int i,
                                         aiTextureType type,
                                         std::shared_ptr<CTexture>& texture) {
  texture = nullptr;
  if (material->GetTextureCount(type) > 0) {
    aiString ai_path;
    if (material->GetTexture(type, 0, &ai_path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
      std::string p(ai_path.data);
      std::string full_path = filedir + "/" + p;

      texture = std::make_shared<CTexture>(full_path, CTexture::ETarget::kTexture2D);
      if (!texture) {
        GE_ERROR("Error loading texture '{0}'", full_path);
        return false;
      }
      GE_INFO("Loaded texture '{0}' at index '{1}'", full_path, i);
    }
  }
  return true;
}

// suppose our mesh contains at most 1 texture 
void GEngine::CMesh::Render(std::shared_ptr<GEngine::CShader> shader) {
  glBindVertexArray(VAO_);
  for (int i = 0; i < meshes_.size(); i++) {
    auto material_index = meshes_[i].material_index_;
    if (materials_[material_index]->diffuse_texture_ != nullptr) {
      shader->SetTexture("texture_diffuse", materials_[material_index]->diffuse_texture_);
    }
    if (materials_[material_index]->normal_texture_ != nullptr) {
      shader->SetTexture("texture_normal", materials_[material_index]->normal_texture_);
    }
    glDrawElementsBaseVertex(GL_TRIANGLES,
                            meshes_[i].num_indices_,
                            GL_UNSIGNED_INT,
                            (void *)(sizeof(unsigned int) * meshes_[i].base_index_),
                            meshes_[i].base_vertex_);
  }
  glBindVertexArray(0);
}

void GEngine::CMesh::Clear() {
  loaded_textures_.clear();
  
  if (VAO_ != 0) {
    glDeleteVertexArrays(1, &VAO_);
    VAO_ = 0;
  }
  if (buffers_[0] != 0) {
    for (int i = 0; i < NUM_BUFFERS; i++) {
      buffers_[i] = 0;
    }
  }
}
