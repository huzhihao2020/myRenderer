#include <iostream>

#include "GEngine.h"

using namespace GEngine;

int main() {

  // register RenderPass & add Objects(todo)
  CSingleton<CRenderSystem>()->RegisterRenderPass(std::make_shared<CSkyboxPass>("skybox_pass", 1));
  CSingleton<CRenderSystem>()->RegisterRenderPass(std::make_shared<CIBLPass>("ibl_pass", 2));

  CSingleton<CApp>()->Init();
  CSingleton<CApp>()->RunMainLoop();
  return 0;
}
