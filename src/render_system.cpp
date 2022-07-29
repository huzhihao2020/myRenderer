#include "render_system.h"

GEngine::CRenderSystem::~CRenderSystem()
{
}

GEngine::CRenderSystem::CRenderSystem()
{
}

void GEngine::CRenderSystem::Init()
{
  render_objects_.clear();
  // set up main camera
  if(!main_camera_) {
    main_camera_ = std::make_shared<CCamera>();
  }
  // todo: setup GUI
  // todo: register Callback
}

std::shared_ptr<GEngine::CGLFWWindow> GEngine::CRenderSystem::GetOrCreateWindow()
{
  if(!window_) {
    window_ = std::make_shared<GEngine::CGLFWWindow>();
  }
  return window_;
}

std::shared_ptr<GEngine::CCamera> GEngine::CRenderSystem::GetOrCreateMainCamera()
{
  if(!main_camera_) {
    main_camera_ = std::make_shared<GEngine::CCamera>();
  }
  return main_camera_;
}

void GEngine::CRenderSystem::RenderCube(CShader& shader)
{
  shader.Use();
  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = GetOrCreateMainCamera()->GetViewMatrix();
  glm::mat4 projection = GetOrCreateMainCamera()->GetProjectionMatrix();
  glm::mat4 projection_view_model = projection * view * model;
  shader.SetMat4("projection_view_model", projection_view_model);
  if(cube_VAO_ != 0) {
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
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
  
  glEnableVertexAttribArray(0); // pos
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)0);
  glEnableVertexAttribArray(1); // normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(3*sizeof(GL_FLOAT)));
  glEnableVertexAttribArray(2); // uv
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(6*sizeof(GL_FLOAT)));
  
  glBindVertexArray(cube_VAO_);
  glEnable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  // reset
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
