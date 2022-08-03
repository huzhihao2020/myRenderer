#include <iostream>

#include "app.h"
#include "render_system.h"
#include "renderpass/skybox_pass.h"

using namespace GEngine;

int main() {

  // register RenderPass & add Objects(todo)
  CSingleton<CRenderSystem>()->RegisterRenderPass(std::make_shared<CSkyboxPass>("skybox_pass", 1));

  CSingleton<CApp>()->Init();
  CSingleton<CApp>()->RunMainLoop();
  return 0;
}
