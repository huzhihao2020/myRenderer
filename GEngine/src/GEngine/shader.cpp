#include "shader.h"
#include "log.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

GEngine::Shader::Shader() {}

GEngine::Shader::Shader(const std::string &vert_shader_path,
                          const std::string &frag_shader_path,
                          const std::string &geom_shader_path) {
  // TODO:
  // tessellation_evaluation_shader, tessellation_control_shader, compute_shader_path
  // compute shader
  std::string vert_code;
  std::string frag_code;
  std::string geom_code;
  std::ifstream vert_shader_file;
  std::ifstream frag_shader_file;
  std::ifstream geom_shader_file;
  // ensure ifstream objects can throw exceptions:
  vert_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  frag_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  geom_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    vert_shader_file.open(vert_shader_path);
    frag_shader_file.open(frag_shader_path);
    std::stringstream vert_shader_stream;
    std::stringstream frag_shader_stream;
    vert_shader_stream << vert_shader_file.rdbuf();
    frag_shader_stream << frag_shader_file.rdbuf();
    vert_shader_file.close();
    frag_shader_file.close();
    vert_code = vert_shader_stream.str();
    frag_code = frag_shader_stream.str();
    if (!geom_shader_path.empty()) {
      geom_shader_file.open(geom_shader_path);
      std::stringstream geom_shader_stream;
      geom_shader_stream << geom_shader_file.rdbuf();
      geom_shader_file.close();
      geom_code = geom_shader_stream.str();
    }
  } catch (std::ifstream::failure &e) {
    GE_ERROR("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: {}", e.what());
  }
  // 2. compile shaders
  unsigned int vertex, fragment;
  const char* vert_c_str = vert_code.c_str();
  const char* frag_c_str = frag_code.c_str();
  // vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vert_c_str, NULL);
  glCompileShader(vertex);
  CheckCompileErrors(vertex, "VERTEX");
  // fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &frag_c_str, NULL);
  glCompileShader(fragment);
  CheckCompileErrors(fragment, "FRAGMENT");
  // if geometry shader is given, compile geometry shader
  unsigned int geometry;
  if (!geom_shader_path.empty()) {
    const char * geom_c_str = geom_code.c_str();
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &geom_c_str, NULL);
    glCompileShader(geometry);
    CheckCompileErrors(geometry, "GEOMETRY");
  }
  // shader Program
  shader_program_ID_ = glCreateProgram();
  glAttachShader(shader_program_ID_, vertex);
  glAttachShader(shader_program_ID_, fragment);
  if (!geom_shader_path.empty())
    glAttachShader(shader_program_ID_, geometry);
  glLinkProgram(shader_program_ID_);
  CheckCompileErrors(shader_program_ID_, "PROGRAM");
  // delete the shaders as they're linked into our program now and no longer necessary
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  if (!geom_shader_path.empty())
    glDeleteShader(geometry);
}

std::shared_ptr<GEngine::Shader>
GEngine::Shader::CreateProgramFromSource(const std::string &vertex_shader_source,
                                 const std::string &fragment_shader_source,
                                 const std::string &geometry_shader_source) {
  auto program = std::make_shared<Shader>();

  if(vertex_shader_source.empty() || fragment_shader_source.empty()){
    GE_ERROR("vertex or fragment sahder not completed");
  }

  unsigned int vertex, fragment, geometry;
  const char* vert_c_str = vertex_shader_source.c_str();
  const char* frag_c_str = fragment_shader_source.c_str();
  // vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vert_c_str, NULL);
  glCompileShader(vertex);
  CheckCompileErrors(vertex, "VERTEX");
  // fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &frag_c_str, NULL);
  glCompileShader(fragment);
  CheckCompileErrors(fragment, "FRAGMENT");
  // if geometry shader is given, compile geometry shader
  if (!geometry_shader_source.empty()) {
    const char * geom_c_str = geometry_shader_source.c_str();
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &geom_c_str, NULL);
    glCompileShader(geometry);
    CheckCompileErrors(geometry, "GEOMETRY");
  }
  // shader Program
  program->shader_program_ID_ = glCreateProgram();
  glAttachShader(program->shader_program_ID_, vertex);
  glAttachShader(program->shader_program_ID_, fragment);
  if (!geometry_shader_source.empty())
    glAttachShader(program->shader_program_ID_, geometry);
  glLinkProgram(program->shader_program_ID_);
  CheckCompileErrors(program->shader_program_ID_, "PROGRAM");
  // delete the shaders as they're linked into our program now and no longer necessary
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  if (!geometry_shader_source.empty())
    glDeleteShader(geometry);
  return program;
}

std::shared_ptr<GEngine::Shader>
GEngine::Shader::CreateProgramFromFile(const std::string &vertex_shader_path,
                                       const std::string &fragment_shader_path,
                                       const std::string &geometry_shader_path) {
  auto program = std::make_shared<Shader>(vertex_shader_path, fragment_shader_path, geometry_shader_path);
  return program;
}

std::shared_ptr<GEngine::Shader>
GEngine::Shader::CreateAtmosphereProgram(const std::string &vertex_shader_path,
                                         const std::string &fragment_shader_path,
                                         const std::string &atmosphere_shader_path) {
  auto program = std::make_shared<Shader>();

  std::string vert_code;
  std::string frag_code;
  std::string frag2_code;
  std::ifstream vert_shader_file;
  std::ifstream frag_shader_file;
  std::ifstream frag2_shader_file;
  // ensure ifstream objects can throw exceptions:
  vert_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  frag_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  frag2_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    vert_shader_file.open(vertex_shader_path);
    frag_shader_file.open(fragment_shader_path);
    frag2_shader_file.open(atmosphere_shader_path);
    std::stringstream vert_shader_stream;
    std::stringstream frag_shader_stream;
    std::stringstream frag2_shader_stream;
    vert_shader_stream << vert_shader_file.rdbuf();
    frag_shader_stream << frag_shader_file.rdbuf();
    frag2_shader_stream << frag2_shader_file.rdbuf();
    vert_shader_file.close();
    frag_shader_file.close();
    frag2_shader_file.close();
    vert_code = vert_shader_stream.str();
    frag_code = frag_shader_stream.str();
    frag2_code = frag2_shader_stream.str();
  } catch (std::ifstream::failure &e) {
    GE_ERROR("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: {}", e.what());
  }

  unsigned int vertex, fragment, fragment2;

  const char* vert_c_str = vert_code.c_str();
  const char* frag_c_str = frag_code.c_str();
  const char* frag2_c_str = frag2_code.c_str();

  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vert_c_str, NULL);
  glCompileShader(vertex);
  CheckCompileErrors(vertex, "VERTEX");
  // fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &frag_c_str, NULL);
  glCompileShader(fragment);
  CheckCompileErrors(fragment, "FRAGMENT");
  // fragment2 Shader
  fragment2 = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment2, 1, &frag2_c_str, NULL);
  glCompileShader(fragment2);
  CheckCompileErrors(fragment2, "FRAGMENT");

  program->shader_program_ID_ = glCreateProgram();
  glAttachShader(program->shader_program_ID_, vertex);
  glAttachShader(program->shader_program_ID_, fragment);
  glAttachShader(program->shader_program_ID_, fragment2);

  glLinkProgram(program->shader_program_ID_);
  CheckCompileErrors(program->shader_program_ID_, "PROGRAM");

  glDeleteShader(vertex);
  glDeleteShader(fragment);
  glDeleteShader(fragment2);

  return program;
}

void GEngine::Shader::Use() const {
  glUseProgram(shader_program_ID_);
  ActiveBoundTextures();
}

void GEngine::Shader::SetBool(const std::string &name, bool value) const {
  glUniform1i(glGetUniformLocation(shader_program_ID_, name.c_str()), (int)value);
}

void GEngine::Shader::SetInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(shader_program_ID_, name.c_str()), value);
}

void GEngine::Shader::SetFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(shader_program_ID_, name.c_str()), value);
}

void GEngine::Shader::SetVec2(const std::string &name,
                              const glm::vec2 &value) const {
  glUniform2fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, &value[0]);
}
void GEngine::Shader::SetVec2(const std::string &name, float x, float y) const {
  glUniform2f(glGetUniformLocation(shader_program_ID_, name.c_str()), x, y);
}

void GEngine::Shader::SetVec3(const std::string &name,
                              const glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, &value[0]);
}
void GEngine::Shader::SetVec3(const std::string &name, float x, float y,
                              float z) const {
  glUniform3f(glGetUniformLocation(shader_program_ID_, name.c_str()), x, y, z);
}

void GEngine::Shader::SetVec4(const std::string &name,
                              const glm::vec4 &value) const {
  glUniform4fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, &value[0]);
}
void GEngine::Shader::SetVec4(const std::string &name, float x, float y,
                              float z, float w) const {
  glUniform4f(glGetUniformLocation(shader_program_ID_, name.c_str()), x, y, z, w);
}

void GEngine::Shader::SetMat2(const std::string &name,
                              const glm::mat2 &mat) const {
  glUniformMatrix2fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

void GEngine::Shader::SetMat3(const std::string &name,
                              const glm::mat3 &mat) const {
  glUniformMatrix3fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

void GEngine::Shader::SetMat4(const std::string &name,
                              const glm::mat4 &mat) const {
  glUniformMatrix4fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

void GEngine::Shader::SetTexture(const std::string &name, const std::shared_ptr<GEngine::CTexture> texture) {
  glUseProgram(shader_program_ID_);
  if (bound_textures_.find(name) != bound_textures_.end()) {
    // [name] already exists, overwrite with new texture
    auto &item = bound_textures_[name];
    std::get<1>(item) = texture;
    glActiveTexture(GL_TEXTURE0 + std::get<0>(item));
    glBindTexture(static_cast<GLuint>(texture->GetTarget()), texture->id_);
  } else {
    // register a new texture
    int binding_slot_index = bound_textures_num_;
    glActiveTexture(GL_TEXTURE0 + binding_slot_index);
    auto uniform_location = glGetUniformLocation(shader_program_ID_, name.c_str());
    glUniform1i(uniform_location, binding_slot_index);
    glBindTexture(static_cast<GLenum>(texture->GetTarget()), texture->id_);
    bound_textures_[name] = std::make_tuple(binding_slot_index, texture, uniform_location);
    bound_textures_num_++;
  }
}

unsigned int GEngine::Shader::GetShaderID() const {
  return shader_program_ID_;
}

void GEngine::Shader::CheckCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      GE_ERROR("ERROR::SHADER_COMPILATION_ERROR of type: {0} \n{1}"
               "-------------------------------------------------------",
               type, infoLog);
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      GE_ERROR("ERROR::SHADER_COMPILATION_ERROR of type: {0} \n{1}"
               "-------------------------------------------------------",
               type, infoLog);
    }
  }
}

void GEngine::Shader::ActiveBoundTextures() const {
  for (const auto item : bound_textures_) {
    glActiveTexture(GL_TEXTURE0 + std::get<0>(item.second));
    const auto &texture = std::get<1>(item.second);
    glBindTexture(static_cast<GLenum>(texture->GetTarget()), texture->id_);
  }
}
