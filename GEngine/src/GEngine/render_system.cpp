#include "GEngine/render_system.h"
#include "log.h"
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

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
    main_camera_->Init();
  }
  // set up main UI
  if (!main_UI_) {
    main_UI_ = std::make_shared<CEditorUI>();
    main_UI_->Init();
  }
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

std::shared_ptr<GEngine::CEditorUI>
GEngine::CRenderSystem::GetOrCreateMainUI() {
  if (!main_UI_) {
    main_UI_ = std::make_shared<GEngine::CEditorUI>();
  }
  return main_UI_;
}

std::any& GEngine::CRenderSystem::GetAnyDataByName(const std::string& name) {
  if(resource_center_.find(name) == resource_center_.end()) {
    GE_ERROR("'{0}' not exists in resource center.", name);
    // fixme
  }
  return resource_center_[name];
}

// std::shared_ptr<GEngine::CModel> &
// GEngine::CRenderSystem::GetOrCreateModelByPath(const std::string &path) {
//   std::string::size_type pos = (path.find_last_of('\\') + 1) == 0
//                                    ? path.find_last_of('/') + 1
//                                    : path.find_last_of('\\') + 1;
//   auto filename = path.substr(pos + 1);
//   if (model_map_.find(filename) == model_map_.end()) {
//     model_map_[filename] = std::make_shared<GEngine::CModel>(path);
//   }
//   return model_map_[filename];
// }

void GEngine::CRenderSystem::SetRenderPipelineType(ERenderPipelineType type) {
  render_pipeline_type_ = type;
}

void GEngine::CRenderSystem::RenderCube() {
  glBindVertexArray(GetOrCreateCubeVAO());
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

int GEngine::CRenderSystem::GetOrCreateCubeVAO() {
  if(cube_VAO_ != 0) {
      return cube_VAO_;
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

  cube_VAO_ = CreateVAO(cube_vertices, sizeof(cube_vertices), {3, 3, 2});
  return cube_VAO_;
}

int GEngine::CRenderSystem::CreateVAO(const GLvoid *vertex_data, int data_size,
              std::initializer_list<int> attribute_layout, const int indices[],
              int indices_size, int *voVBO) {
  unsigned int VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, data_size, vertex_data, GL_STATIC_DRAW);
  if(indices) {
    // todo
  }
  int offset = 0;
  int i = 0;
  int stride = std::accumulate(attribute_layout.begin(), attribute_layout.end(), 0);
  for(auto length : attribute_layout) {
      glEnableVertexAttribArray(i);
    glVertexAttribPointer(i, length, GL_FLOAT, GL_FALSE, stride * sizeof(GL_FLOAT), (GLvoid*)(offset * sizeof(GL_FLOAT)));
    offset += length;
    i++;
  }
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  if(voVBO) {
    // todo
  }
  return VAO;
}

void GEngine::CRenderSystem::RenderSphere() {
  if (sphere_VAO_ == 0) {
    glGenVertexArrays(1, &sphere_VAO_);

    unsigned int vbo, ebo;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> tangents;
    std::vector<unsigned int> indices;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
      for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
        float xSegment = (float)x / (float)X_SEGMENTS;
        float ySegment = (float)y / (float)Y_SEGMENTS;
        float phi = xSegment * 2.0f * PI;
        float theta = ySegment * PI;
        float xPos = std::cos(phi) * std::sin(theta);
        float yPos = std::cos(theta);
        float zPos = std::sin(phi) * std::sin(theta);

        positions.push_back(glm::vec3(xPos, yPos, zPos));
        normals.push_back(glm::vec3(xPos, yPos, zPos));
        uv.push_back(glm::vec2(xSegment, ySegment));
        tangents.push_back(glm::vec3(zPos, 0, -xPos));
      }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
      if (!oddRow) // even rows: y == 0, y == 2; and so on
      {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
          indices.push_back(y * (X_SEGMENTS + 1) + x);
          indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
        }
      } else {
        for (int x = X_SEGMENTS; x >= 0; --x) {
          indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
          indices.push_back(y * (X_SEGMENTS + 1) + x);
        }
      }
      oddRow = !oddRow;
    }
    sphere_index_count_ = static_cast<unsigned int>(indices.size());

    std::vector<float> data;
    for (unsigned int i = 0; i < positions.size(); ++i) {
      data.push_back(positions[i].x);
      data.push_back(positions[i].y);
      data.push_back(positions[i].z);
      if (normals.size() > 0) {
        data.push_back(normals[i].x);
        data.push_back(normals[i].y);
        data.push_back(normals[i].z);
      }
      if (uv.size() > 0) {
        data.push_back(uv[i].x);
        data.push_back(uv[i].y);
      }
      if(tangents.size() > 0) {
        data.push_back(tangents[i].x);
        data.push_back(tangents[i].y);
        data.push_back(tangents[i].z);
      }
    }
    
    glBindVertexArray(sphere_VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    unsigned int stride = (3 + 3 + 2 + 3) * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void *)(8 * sizeof(float)));
  }

  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(sphere_VAO_);
  glDrawElements(GL_TRIANGLE_STRIP, sphere_index_count_, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
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
      GE_ERROR("Texture format error: {}", path);
      stbi_image_free(data);
    }
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

  } else {
    GE_ERROR("Texture failed to load at path: {}", path);
    stbi_image_free(data);
  }
  return textureID;
}

void GEngine::CRenderSystem::RegisterRenderPass(
    const std::shared_ptr<GEngine::CRenderPass> &render_pass) {
  if (render_pass == nullptr) {
    return;
  }
  render_passes_.push_back(render_pass);
  std::sort(render_passes_.begin(), render_passes_.end(),
            [](const std::shared_ptr<GEngine::CRenderPass> render_pass1,
               const std::shared_ptr<GEngine::CRenderPass> render_pass2) {
              return render_pass1 < render_pass2;
            });
  // render_passes_.insert(
  //     std::lower_bound(
  //         render_passes_.begin(), render_passes_.end(), render_pass,
  //         [](const std::shared_ptr<GEngine::CRenderPass> render_pass1,
  //            const std::shared_ptr<GEngine::CRenderPass> render_pass2) {
  //           render_pass1 < render_pass2
  //         }),
  //     render_pass);
}

void GEngine::CRenderSystem::RegisterAnyDataWithName(const std::string& name, std::any data) {
  if(!data.has_value()) {
    GE_ERROR("Failed to register data {0}.", name);
    return;
  }
  if(resource_center_.find(name)!=resource_center_.end()) {
    GE_WARN("Registering data {0} already exist in resource center", name);
  }

  GE_INFO("Register data '{0}' to resource center", name);
  resource_center_[name] = data;
}
