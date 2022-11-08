#pragma once
#include <glm/glm.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "GEngine/texture.h"
#include <vector>
#include <tuple>
#include <map>
#include <array>

namespace GEngine {

class Shader {
public:
  Shader();
  Shader(const std::string& vertex_shader_path,
          const std::string& fragment_shader_path,
          const std::string& geometry_shader_path = "");

  static std::shared_ptr<Shader>
  CreateProgramFromSource(const std::string &vertex_shader_source,
                          const std::string &fragment_shader_source,
                          const std::string &geometry_shader_source = "");
  static std::shared_ptr<Shader>
  CreateProgramFromFile(const std::string &vertex_shader_path,
                        const std::string &fragment_shader_path,
                        const std::string &geometry_shader_path = "");

  static std::shared_ptr<Shader>
  CreateAtmosphereProgram(const std::string &vertex_shader_path,
                          const std::string &fragment_shader_path,
                          const std::string &atmosphere_shader_path);

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
  static void CheckCompileErrors(GLuint shader, std::string type);
  void ActiveBoundTextures() const;

  unsigned int shader_program_ID_ = 0;
  // number of textures bound to specific shader instance
  int bound_textures_num_ = 0; 
  // ["textureName": texture_slot, texturePtr, uniform_location] for each  bound texture
  std::map<std::string, std::tuple<int, std::shared_ptr<GEngine::CTexture>, int>> bound_textures_;
};

// todo: refractoring Shader & Program
class Program {
 public:
  Program(const std::string& vertex_shader_source, const std::string& fragment_shader_source)
    : Program(vertex_shader_source, "", fragment_shader_source) {
  }

  Program(const std::string& vertex_shader_source,
          const std::string& geometry_shader_source,
          const std::string& fragment_shader_source) {
    program_ = glCreateProgram();

    const char* source;
    source = vertex_shader_source.c_str();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &source, NULL);
    glCompileShader(vertex_shader);
    CheckShader(vertex_shader);
    glAttachShader(program_, vertex_shader);

    GLuint geometry_shader = 0;
    if (!geometry_shader_source.empty()) {
      source = geometry_shader_source.c_str();
      geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(geometry_shader, 1, &source, NULL);
      glCompileShader(geometry_shader);
      CheckShader(geometry_shader);
      glAttachShader(program_, geometry_shader);
    }

    source = fragment_shader_source.c_str();
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &source, NULL);
    glCompileShader(fragment_shader);
    CheckShader(fragment_shader);
    glAttachShader(program_, fragment_shader);

    glLinkProgram(program_);
    CheckProgram(program_);

    glDetachShader(program_, vertex_shader);
    glDeleteShader(vertex_shader);
    if (!geometry_shader_source.empty()) {
      glDetachShader(program_, geometry_shader);
      glDeleteShader(geometry_shader);
    }
    glDetachShader(program_, fragment_shader);
    glDeleteShader(fragment_shader);
  }

  ~Program() {
    glDeleteProgram(program_);
  }

  void Use() const {
    glUseProgram(program_);
  }

  void BindMat3(const std::string& uniform_name,
      const std::array<float, 9>& value) const {
    glUniformMatrix3fv(glGetUniformLocation(program_, uniform_name.c_str()),
        1, true /* transpose */, value.data());
  }

  void BindInt(const std::string& uniform_name, int value) const {
    glUniform1i(glGetUniformLocation(program_, uniform_name.c_str()), value);
  }

  void BindTexture2d(const std::string& sampler_uniform_name, GLuint texture,
      GLuint texture_unit) const {
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    BindInt(sampler_uniform_name, texture_unit);
  }

  void BindTexture3d(const std::string& sampler_uniform_name, GLuint texture,
      GLuint texture_unit) const {
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_3D, texture);
    BindInt(sampler_uniform_name, texture_unit);
  }

 private:
  static void CheckShader(GLuint shader) {
    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status == GL_FALSE) {
      PrintShaderLog(shader);
    }
    assert(compile_status == GL_TRUE);
  }

  static void PrintShaderLog(GLuint shader) {
    GLint log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
      std::unique_ptr<char[]> log_data(new char[log_length]);
      glGetShaderInfoLog(shader, log_length, &log_length, log_data.get());
      std::cerr << "compile log = "
                << std::string(log_data.get(), log_length) << std::endl;
    }
  }

  static void CheckProgram(GLuint program) {
    GLint link_status;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
      PrintProgramLog(program);
    }
    assert(link_status == GL_TRUE);
    assert(glGetError() == 0);
  }

  static void PrintProgramLog(GLuint program) {
    GLint log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
      std::unique_ptr<char[]> log_data(new char[log_length]);
      glGetProgramInfoLog(program, log_length, &log_length, log_data.get());
      std::cerr << "link log = "
                << std::string(log_data.get(), log_length) << std::endl;
    }
  }

  GLuint program_;
};

} // namespace GEngine
