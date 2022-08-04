#include <iostream>
#include <memory>
#include <string>
#include "app.h"
#include "render_system.h"
#include "input_system.h"
#include "shader.h"
#include "log.h"
#include "model.h"
#include "texture.h"

GEngine::CApp::CApp()
{
}

GEngine::CApp::~CApp()
{
}

GLvoid GEngine::CApp::Init()
{
  GEngine::CLog::Init();
  CSingleton<CRenderSystem>()->GetOrCreateWindow()->Init();
  window_ = CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow();
  CSingleton<CRenderSystem>()->Init();
  CSingleton<CRenderSystem>()->GetOrCreateMainUI()->Init();
  CSingleton<CInputSystem>()->Init();
  // renderpass init
  for(auto& pass : CSingleton<CRenderSystem>()->GetRenderPass()) {
    pass->Init();
  }
}

GLvoid GEngine::CApp::RunMainLoop() {
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  auto cube_shader  = std::make_shared<CShader>(std::string("../../shaders/vert.glsl"),
                                                std::string("../../shaders/frag.glsl"));
  auto model_shader = std::make_shared<CShader>(std::string("../../shaders/model_VS.glsl"), 
                                                std::string("../../shaders/model_FS.glsl"));
  // texture
  std::string texture_path = "../../assets/textures/marble.jpg";
  auto cube_texture = std::make_shared<CTexture>(texture_path); 
  cube_shader->SetTexture("cube_texture", cube_texture);

  // model
  std::string model_path("../../assets/backpack/backpack.obj");
  auto model_backpack = GEngine::CModel(model_path);

  // init main_ui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto io_ = &ImGui::GetIO(); (void)(*io_);
  io_->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io_->ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
  io_->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  auto style_ = &ImGui::GetStyle();
  if (io_->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style_->WindowRounding = 0.0f;
    style_->Colors[ImGuiCol_WindowBg].w = 1.0f;
    style_->Alpha = 0.8f;
  }
  // Setup Platform/Renderer backends
  
  ImGui_ImplGlfw_InitForOpenGL(CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow(), true);
  ImGui_ImplOpenGL3_Init("#version 330");
  // render loop
  while (!glfwWindowShouldClose(window_)) {
    CalculateTime();
    CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->Tick();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ticking the render passes
    auto render_passes = CSingleton<CRenderSystem>()->GetRenderPass();
    for (int i = 0; i < render_passes.size(); i++) {
      if (render_passes[i]->GetOrder() == -1)
        continue;
      switch (render_passes[i]->GetType()) {
      case GEngine::ERenderPassType::Once:
        render_passes[i]->Tick();
        render_passes[i]->SetOrder(-1);
        break;
      default:
        render_passes[i]->Tick();
        break;
      }
    }
    // ticking the objects
    cube_shader->Use();
    CSingleton<CRenderSystem>()->RenderCube(cube_shader); // render cube
    model_shader->Use();// [render model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2, 2, 2));
    model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
    glm::mat4 view = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetViewMatrix();
    glm::mat4 projection = CSingleton<CRenderSystem>()->GetOrCreateMainCamera()->GetProjectionMatrix();
    glm::mat4 projection_view_model = projection * view * model;
    model_shader->SetMat4("projection_view_model", projection_view_model);
    model_backpack.Draw(model_shader); // render model]

    // ticking main GUI
    CSingleton<CRenderSystem>()->GetOrCreateMainUI()->Tick();
    // todo
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    int w_width, w_height;
    glfwGetWindowSize(CSingleton<CRenderSystem>()->GetOrCreateWindow()->GetGLFWwindow(), &w_width, &w_height);
    // imgui begin
    ImGuiWindowFlags window_flag = 0;
    window_flag |= ImGuiWindowFlags_MenuBar;
    auto show_window_ = true;
    ImGui::Begin("Menu", &show_window_, window_flag);
    ImGui::TextWrapped("Window Size: %d, %d ", w_width, w_height);
    auto camera = CSingleton<CRenderSystem>()->GetOrCreateMainCamera();
    auto camera_position = camera->GetPosition();
    auto camera_front = camera->GetFront();
    ImGui::Text("cameraPos: (%.3f, %.3f, %.3f)", camera_position.x, camera_position.y, camera_position.z);
    ImGui::Text("cameraFront: (%.3f, %.3f, %.3f)", camera_front.x, camera_front.y, camera_front.z);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("[Q]: Enable/Disable Cursor");
    ImGui::Text("[Up/Down]: Change Move Speed of Camera");
    ImGui::Text("[ESC]: Close Window");
    ImGui::Text("io.wantCaptureMouse, %d", int(io_->WantCaptureMouse));
    ImGui::Text("mouse pos: %f, %f", io_->MousePos.x, io_->MousePos.y);
    if (ImGui::Button("Test Button")) {
      printf("Test Button Clicked");
    }
    ImGui::End();
    // imgui end
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io_->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    glfwPollEvents();
    glfwSwapBuffers(window_);
  }
  glfwTerminate();
}

void GEngine::CApp::CalculateTime() {
  currentTime_ = glfwGetTime();
  deltaTime_ = currentTime_ - lastFrameTime_;
  lastFrameTime_ = currentTime_;

  ++frameCounter_;
  deltaTimeCounter_ += deltaTime_;
  if(deltaTimeCounter_ > 1.0) {
    currentFPS_ = frameCounter_;
    frameCounter_ = 0;
    deltaTimeCounter_ = 0.0;
  }
}

GLdouble GEngine::CApp::GetCurrentTime() const {
  return currentTime_;
}

GLdouble GEngine::CApp::GetDeltaTime() const {
  return deltaTime_;
}

GLuint GEngine::CApp::GetFPS() const {
  return currentFPS_;
}
