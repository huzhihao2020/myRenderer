#include "render_system.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

GEngine::CRenderSystem::~CRenderSystem()
{
}

GEngine::CRenderSystem::CRenderSystem()
{
}

void GEngine::CRenderSystem::Init() {
  // set up main camera
  if (!main_camera_) {
    main_camera_ = std::make_shared<CCamera>();
  }
  // todo: setup GUI
  // todo: register Callback
}

std::shared_ptr<GEngine::CGLFWWindow>
GEngine::CRenderSystem::GetOrCreateWindow() {
  if (!window_) {
    window_ = std::make_shared<GEngine::CGLFWWindow>();
  }
  return window_;
}

std::shared_ptr<GEngine::CCamera>
GEngine::CRenderSystem::GetOrCreateMainCamera() {
  if (!main_camera_) {
    main_camera_ = std::make_shared<GEngine::CCamera>();
  }
  return main_camera_;
}

std::shared_ptr<GEngine::CModel> &
GEngine::CRenderSystem::GetOrCreateModelByPath(const std::string &path) {
  std::string::size_type pos = (path.find_last_of('\\') + 1) == 0
                                   ? path.find_last_of('/') + 1
                                   : path.find_last_of('\\') + 1;
  auto filename = path.substr(pos + 1);
  if (model_map_.find(filename) == model_map_.end()) {
    model_map_[filename] = std::make_shared<GEngine::CModel>(path);
  }
  return model_map_[filename];
}

void GEngine::CRenderSystem::RenderCube(CShader &shader) {
  shader.Use();
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = GetOrCreateMainCamera()->GetViewMatrix();
  glm::mat4 projection = GetOrCreateMainCamera()->GetProjectionMatrix();
  glm::mat4 projection_view_model = projection * view * model;
  shader.SetMat4("projection_view_model", projection_view_model);
  if(cube_VAO_ != 0) {
    // cube already exists
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 1);
    glBindVertexArray(cube_VAO_);
    glEnable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    return;
  }
  float cube_vertices[] = {
    // positions          // normal            // uv
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
      1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
      1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
      1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
      1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
      1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
      1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
      1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
      1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
      1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
      1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
      1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
      1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f};

  // setup VAO & VBO
     unsigned int cube_VBO;
     glGenVertexArrays(1, &cube_VAO_);
     glBindVertexArray(cube_VAO_);
     glGenBuffers(1, &cube_VBO);
     glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
     glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices,
                  GL_STATIC_DRAW);

     glEnableVertexAttribArray(0); // pos
     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT),
                           (void *)0);
     glEnableVertexAttribArray(1); // normal
     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT),
                           (void *)(3 * sizeof(GL_FLOAT)));
     glEnableVertexAttribArray(2); // uv
     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT),
                           (void *)(6 * sizeof(GL_FLOAT)));

     glBindVertexArray(cube_VAO_);
     glEnable(GL_DEPTH_TEST);
     glDrawArrays(GL_TRIANGLES, 0, 36);
     // reset
     glBindVertexArray(0);
     glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int GEngine::CRenderSystem::LoadTexture(const std::string &path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  // flip on load
  stbi_set_flip_vertically_on_load(true);

  int width, height, nrComponents;
  unsigned char *data =
      stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1) {
      format = GL_RED;
    } else if (nrComponents == 3) {
      format = GL_RGB;
    } else if (nrComponents == 4) {
      format = GL_RGBA;
    } else {
      std::cout << "Texture format error " << path << std::endl;
      stbi_image_free(data);
    }
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }
  return textureID;
}
