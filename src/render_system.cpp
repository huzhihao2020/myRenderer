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

// void GEngine::CRenderSystem::DrawCube()
// {
//      float cube_vertices[] = {
//         // positions          // normal            // uv
//         -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
//          1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
//          1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
//          1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
//         -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
//         -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

//         -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
//          1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
//          1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
//          1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
//         -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
//         -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

//         -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
//         -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
//         -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
//         -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
//         -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
//         -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

//         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
//         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
//         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
//         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
//         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
//         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

//         -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
//          1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
//          1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
//          1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
//         -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
//         -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

//         -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
//          1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
//          1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
//          1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
//         -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
//         -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f};

//   unsigned int cube_VAO, cube_VBO;
//   glGenVertexArrays(1, &cube_VAO);
//   glGenBuffers(1, &cube_VBO);
//   glBindVertexArray(cube_VAO);
//   glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
//   glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
//   glEnableVertexAttribArray(0);
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)0);
//   glEnableVertexAttribArray(1);
//   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(3*sizeof(GL_FLOAT)));
//   glEnableVertexAttribArray(2);
//   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (void*)(6*sizeof(GL_FLOAT)));

//   glBindVertexArray(cube_VAO);
//   glDrawArrays(GL_TRIANGLES, 0, 36);
//   glBindVertexArray(0);
// }
