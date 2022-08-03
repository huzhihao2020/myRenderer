#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "texture.h"
#include <vector>
#include <tuple>
#include <map>

namespace GEngine {

class CShader {
public:
  CShader(const std::string& vertex_shader_path,
          const std::string& fragment_shader_path,
          const std::string& geometry_shader_path = "");

  void Use() const;
  void SetBool(const std::string &name, bool value) const;
  void SetInt(const std::string &name, int value) const;
  void SetFloat(const std::string &name, float value) const;
  void SetVec2(const std::string &name, const glm::vec2 &value) const;
  void SetVec2(const std::string &name, float x, float y) const;
  void SetVec3(const std::string &name, const glm::vec3 &value) const;
  void SetVec3(const std::string &name, float x, float y, float z) const;
  void SetVec4(const std::string &name, const glm::vec4 &value) const;
  void SetVec4(const std::string &name, float x, float y, float z, float w) const;
  void SetMat2(const std::string &name, const glm::mat2 &mat) const;
  void SetMat3(const std::string &name, const glm::mat3 &mat) const;
  void SetMat4(const std::string &name, const glm::mat4 &mat) const;
  void SetTexture(const std::string &name, const std::shared_ptr<GEngine::CTexture> texture);

  unsigned int GetShaderID() const;
private:
  void CheckCompileErrors(GLuint shader, std::string type);
  void ActiveBoundTextures() const;

  unsigned int shader_program_ID_ = 0;
  // number of textures bound to specific shader instance
  int bound_textures_num_ = 0; 
  // ["textureName": textureID, texturePtr] for each  bound texture
  std::map<std::string, std::tuple<int, std::shared_ptr<GEngine::CTexture>>> bound_textures_;
};
} // namespace GEngine
