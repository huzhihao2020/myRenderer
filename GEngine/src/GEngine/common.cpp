#include "GEngine/common.h"

#include<GLFW/glfw3.h>

#include <iostream>
#include <fstream>

int GEngine::WINDOW_CONFIG::WINDOW_WIDTH = 1280;
int GEngine::WINDOW_CONFIG::WINDOW_HEIGHT = 720;
int GEngine::WINDOW_CONFIG::VIEWPORT_LOWERLEFT_X = 0;
int GEngine::WINDOW_CONFIG::VIEWPORT_LOWERLEFT_Y = 0;
int GEngine::WINDOW_CONFIG::VIEWPORT_WIDTH = GEngine::WINDOW_CONFIG::WINDOW_WIDTH;
int GEngine::WINDOW_CONFIG::VIEWPORT_HEIGHT = GEngine::WINDOW_CONFIG::WINDOW_HEIGHT;
std::string GEngine::WINDOW_CONFIG::WINDOW_TITLE = "GEngine";

// TRUE if on mac retina screen, fix me
bool GEngine::WINDOW_CONFIG::IS_MACOS_WINDOW = true;

void SaveShaderToFile(const GLuint shader, const std::string& filename) {
  GLint sourceLength;
  glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &sourceLength);

  GLsizei actualLength;
  std::unique_ptr<GLchar[]> buffer(new GLchar[sourceLength]);
  glGetShaderSource(shader, sourceLength, &actualLength, buffer.get());

  std::ofstream output_stream(filename, std::ofstream::out);
  output_stream << std::string(buffer.get());
  output_stream.close();
}

void SaveTexture(const GLenum texture_unit, const GLenum texture_target,
    const int texture_size, const std::string& filename) {
  std::unique_ptr<float[]> pixels(new float[texture_size * 4]);
  glActiveTexture(texture_unit);
  glGetTexImage(texture_target, 0, GL_RGBA, GL_FLOAT, pixels.get());

  std::ofstream output_stream(
      filename, std::ofstream::out | std::ofstream::binary);
  output_stream.write((const char*) pixels.get(), texture_size * 16);
  output_stream.close();
}

// void SaveTextureToImage(const GLenum texture_unit, const GLenum texture_target,
//     const int width, const int height, const std::string& filename) {
//   std::unique_ptr<float[]> pixels(new float[texture_size * 4]);
//   glActiveTexture(texture_unit);
//   glGetTexImage(texture_target, 0, GL_RGBA, GL_FLOAT, pixels.get());

//   stbi_write_bmp( "myfile.bmp", width, height, 4, pixels );
// }
