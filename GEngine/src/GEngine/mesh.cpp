#include "GEngine/mesh.h"
#include "GEngine/log.h"
#include "GEngine/material.h"
#include "GEngine/shader.h"
#include "assimp/GltfMaterial.h"
#include "assimp/material.h"
#include "assimp/types.h"
#include "common.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include <tuple>
#include <glm/gtc/type_ptr.hpp>

#define POISITION_LOCATION  0
#define NORMAL_LOCATION     1
#define TEX_COORD_LOCATION  2
#define TANGENT_LOCATION    3
#define BONE_ID             4
#define WEIGHTS             5

#define MAX_BONE_INFLUENCE 4
#define MAX_TOTAL_BONE 200

GEngine::CMesh::CMesh() {}

GEngine::CMesh::~CMesh() {}

bool GEngine::CMesh::LoadMesh(const std::string &filename) {
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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), &indices_[0], GL_STATIC_DRAW);

  Assimp::Importer importer;
  auto flags = aiProcess_Triangulate 
               | aiProcess_GenSmoothNormals
               | aiProcess_FlipUVs 
               | aiProcess_CalcTangentSpace
               | aiProcess_JoinIdenticalVertices;
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

bool GEngine::CMesh::InitFromScene(const aiScene* scene, const std::string &filename) {
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
  bone_ids_.reserve(total_vertices);
  weights_.reserve(total_vertices);
  indices_.reserve(total_indices);

  // traverse all meshes and init them
  for(int i=0; i<meshes_.size(); i++) {
    const aiVector3D zero3f(0.0f, 0.0f, 0.0f);
    const aiVector3D default_normal(0.0f, 1.0f, 0.0f);
    const aiVector3D default_tangent(0.0f, 0.0f, 1.0f);
    const aiVector3D default_bitangent(1.0f, 0.0f,0.0f);
    auto ai_mesh = scene->mMeshes[i];
    for(int jj=0; jj<ai_mesh->mNumVertices; jj++) {
      const auto& a_pos = ai_mesh->HasPositions() ? ai_mesh->mVertices[jj] : zero3f;
      const auto& a_normal = ai_mesh->HasNormals() ? ai_mesh->mNormals[jj] : default_normal;
      const auto& a_texcoord = ai_mesh->HasTextureCoords(0) ? ai_mesh->mTextureCoords[0][jj] : zero3f;
      const auto& a_tangent = ai_mesh->HasTangentsAndBitangents() ? ai_mesh->mTangents[jj] : default_tangent;
      
      //init bone info for vertices
      std::vector<int> a_bone_id;
      std::vector<float> a_bone_weight;

      positions_.push_back(glm::vec3(a_pos.x, a_pos.y, a_pos.z));
      normals_.push_back(glm::vec3(a_normal.x, a_normal.y, a_normal.z));
      texcoords_.push_back(glm::vec2(a_texcoord.x, a_texcoord.y));
      tangents_.push_back(glm::vec3(a_tangent.x, a_tangent.y, a_tangent.z));
      bone_ids_.push_back(glm::ivec4(-1, -1, -1, -1));
      weights_.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    }

    // 遍历bone的信息，用bone_info_[bone_name, {id, inv_binding_matrix}]存起来
    for (int boneIndex = 0; boneIndex < ai_mesh->mNumBones; ++boneIndex) {
      int boneID = -1;
      std::string boneName = ai_mesh->mBones[boneIndex]->mName.C_Str();
      if(bone_info_.find(boneName)!=bone_info_.end()) {
        boneID = bone_info_[boneName]->id;
      }
      else {
        auto tmp_bone_info  = std::make_shared<SBoneInfo>();
        tmp_bone_info->id = bone_counter_;
        // offset matrix 就是 inverse_bind_transform
        tmp_bone_info->inverse_bind_transform = GEngine::Utils::ConvertMatrixToGLMFormat(ai_mesh->mBones[boneIndex]->mOffsetMatrix);
        bone_info_[boneName] = tmp_bone_info;
        boneID = bone_counter_;
        bone_counter_++;
      }
      assert(boneID != -1);
      
      // boneID骨骼影响到了多少顶点
      auto weights = ai_mesh->mBones[boneIndex]->mWeights;
      int numWeights = ai_mesh->mBones[boneIndex]->mNumWeights;
      for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
        int vertexId = weights[weightIndex].mVertexId + meshes_[i].base_vertex_;
        float weight = weights[weightIndex].mWeight;
        assert(vertexId <= total_vertices);
        //SetVertexBoneData(vertices[vertexId], boneID, weight);
        for (int k= 0; k < MAX_BONE_INFLUENCE; k++) {
          if (bone_ids_[vertexId][k] < 0 && weight > 0) {
            weights_[vertexId][k] = weight;
            bone_ids_[vertexId][k] = boneID;
            break;
          }
        }
      }
    }

    for(int kk=0; kk<ai_mesh->mNumFaces; kk++) {
      const auto& face = ai_mesh->mFaces[kk];
      if(face.mNumIndices != 3) {
        GE_ERROR("face indices number is not 3!");
        assert(0);
      }
      indices_.push_back(face.mIndices[0]);
      indices_.push_back(face.mIndices[1]);
      indices_.push_back(face.mIndices[2]);
    }
    num_faces_ += ai_mesh->mNumFaces;;
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

  glBindBuffer(GL_ARRAY_BUFFER, buffers_[TANGENT]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tangents_[0]) * tangents_.size(), &tangents_[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(TANGENT_LOCATION);
  glVertexAttribPointer(TANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, buffers_[BONE_ID]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(bone_ids_[0]) * bone_ids_.size(), &bone_ids_[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(BONE_ID);
  glVertexAttribIPointer(BONE_ID, 4, GL_INT, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, buffers_[WEIGHTS]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(weights_[0]) * weights_.size(), &weights_[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(WEIGHTS);
  glVertexAttribPointer(WEIGHTS, 4, GL_FLOAT, GL_FALSE, 0, 0);

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

  // load material colors and textures
  // todo: check embbeded texture / separate texture
  for (int idx = 0; idx < scene->mNumMaterials; idx++) {
    const aiMaterial *p_material = scene->mMaterials[idx];
    aiColor4D color (0.f,0.f,0.f,0.f);

    if(aiGetMaterialColor(p_material, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS) {
      materials_[idx]->mat_desc_.has_base_color = true;
      materials_[idx]->basecolor_ = glm::vec3(color.r, color.g, color.b);
    }
  
    // diffuse & basecolor texture
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_DIFFUSE, materials_[idx]->diffuse_texture_);
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_BASE_COLOR, materials_[idx]->basecolor_texture_);
    // some models mess up HeightMap and NormalMap
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_HEIGHT, materials_[idx]->normal_texture_);
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_NORMALS, materials_[idx]->normal_texture_);
    // maybe the [alpha] channel in diffuse texture
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_OPACITY, materials_[idx]->alpha_texture_); 
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_DIFFUSE_ROUGHNESS, materials_[idx]->roughness_texture_);
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_METALNESS, materials_[idx]->metallic_texture_);
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_AMBIENT_OCCLUSION, materials_[idx]->ao_texture_);
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_EMISSION_COLOR, materials_[idx]->emissive_texture_);
    // roughness-metallic for glTF format (g,b channel)
    LoadMaterialTexture(p_material, filedir, idx, aiTextureType_UNKNOWN, materials_[idx]->unknown_texture_);
  }
  return true;
}

bool GEngine::CMesh::LoadMaterialTexture(const aiMaterial *material,
                                         std::string &filedir,
                                         int material_index,
                                         aiTextureType type,
                                         std::shared_ptr<CTexture>& texture) {
  texture = nullptr;
  if (material->GetTextureCount(type) > 0) {
    aiString ai_path;
    if (material->GetTexture(type, 0, &ai_path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
      std::string p(ai_path.data);

      // replace "\\" in p with '/'
      for(int i=0; i<p.length(); i++) {
        if(p[i]=='\\') {
          p[i] = '/';
        }
      }

      std::string full_path = filedir + "/" + p;

      texture = std::make_shared<CTexture>(full_path, CTexture::ETarget::kTexture2D);
      if (!texture) {
        GE_ERROR("Error loading texture '{0}' at index '{1}'", full_path, material_index);
        return false;
      }
      GE_INFO("Load texture '{0}' at index '{1}'", full_path, material_index);
    }
  }
  return true;
}

// suppose our mesh contains at most 1 texture 
void GEngine::CMesh::Render(std::shared_ptr<GEngine::Shader> shader) {
  glBindVertexArray(VAO_);
  for (int i = 0; i < meshes_.size(); i++) {
    auto material_index = meshes_[i].material_index_;

    // set uniforms
    if(materials_[material_index]->mat_desc_.has_base_color) {
      shader->SetVec3("u_diffuse_color", materials_[material_index]->basecolor_);
    }
    // todo: set uniforms according to mat_desc_

    // common
    shader->SetBool("has_diffuse_texture", false);
    if (materials_[material_index]->diffuse_texture_ != nullptr) {
      shader->SetBool("has_diffuse_texture", true);
      shader->SetTexture("texture_diffuse", materials_[material_index]->diffuse_texture_);
    }
    shader->SetBool("has_normal_texture", false);
    if (materials_[material_index]->normal_texture_ != nullptr) {
      shader->SetBool("has_normal_texture", true);
      shader->SetTexture("texture_normal", materials_[material_index]->normal_texture_);
    } 
    // discard if alpha!=1, blending not implemented
    shader->SetBool("has_alpha_texture", false);
    if (materials_[material_index]->alpha_texture_ != nullptr) {
      shader->SetBool("has_alpha_texture", true);
      shader->SetTexture("texture_alpha", materials_[material_index]->alpha_texture_);
    }
    // pbr
    shader->SetBool("has_base_color_texture", false);
    if (materials_[material_index]->basecolor_texture_ != nullptr) {
      shader->SetBool("has_base_color_texture", true);
      shader->SetTexture("texture_base_color", materials_[material_index]->basecolor_texture_);
    }
    shader->SetBool("has_metallic_texture", false);
    if (materials_[material_index]->metallic_texture_ != nullptr) {
      shader->SetBool("has_metallic_texture", true);
      shader->SetTexture("texture_metallic", materials_[material_index]->metallic_texture_);
    }
    shader->SetBool("has_roughness_texture", false);
    if (materials_[material_index]->roughness_texture_ != nullptr) {
      shader->SetBool("has_roughness_texture", true);
      shader->SetTexture("texture_roughness", materials_[material_index]->roughness_texture_);
    }
    if (materials_[material_index]->ao_texture_ != nullptr) {
      shader->SetTexture("texture_ao", materials_[material_index]->ao_texture_);
    }
    if (materials_[material_index]->emissive_texture_ != nullptr) {
      shader->SetTexture("texture_emissive", materials_[material_index]->emissive_texture_);
    }
    if (materials_[material_index]->unknown_texture_ != nullptr) {
      shader->SetTexture("texture_metallic_roughness", materials_[material_index]->unknown_texture_);
    }

    // set animation uniforms

    glEnable(GL_DEPTH_TEST);
    shader->Use();
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
