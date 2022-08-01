#include "shader.h"

GEngine::CShader::CShader(const std::string &vert_shader_path,
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
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what()
              << std::endl;
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

void GEngine::CShader::Use() const { glUseProgram(shader_program_ID_); }

void GEngine::CShader::SetBool(const std::string &name, bool value) const {
  glUniform1i(glGetUniformLocation(shader_program_ID_, name.c_str()), (int)value);
}

void GEngine::CShader::SetInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(shader_program_ID_, name.c_str()), value);
}

void GEngine::CShader::SetFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(shader_program_ID_, name.c_str()), value);
}

void GEngine::CShader::SetVec2(const std::string &name,
                              const glm::vec2 &value) const {
  glUniform2fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, &value[0]);
}
void GEngine::CShader::SetVec2(const std::string &name, float x, float y) const {
  glUniform2f(glGetUniformLocation(shader_program_ID_, name.c_str()), x, y);
}

void GEngine::CShader::SetVec3(const std::string &name,
                              const glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, &value[0]);
}
void GEngine::CShader::SetVec3(const std::string &name, float x, float y,
                              float z) const {
  glUniform3f(glGetUniformLocation(shader_program_ID_, name.c_str()), x, y, z);
}

void GEngine::CShader::SetVec4(const std::string &name,
                              const glm::vec4 &value) const {
  glUniform4fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, &value[0]);
}
void GEngine::CShader::SetVec4(const std::string &name, float x, float y,
                              float z, float w) const {
  glUniform4f(glGetUniformLocation(shader_program_ID_, name.c_str()), x, y, z, w);
}

void GEngine::CShader::SetMat2(const std::string &name,
                              const glm::mat2 &mat) const {
  glUniformMatrix2fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

void GEngine::CShader::SetMat3(const std::string &name,
                              const glm::mat3 &mat) const {
  glUniformMatrix3fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

void GEngine::CShader::SetMat4(const std::string &name,
                              const glm::mat4 &mat) const {
  glUniformMatrix4fv(glGetUniformLocation(shader_program_ID_, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

unsigned int GEngine::CShader::GetShaderID() const {
  return shader_program_ID_;
}

void GEngine::CShader::CheckCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      std::cout
          << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      std::cout
          << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
  }
}
