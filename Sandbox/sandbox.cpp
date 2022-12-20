#include <iostream>

#include "GEngine.h"

using namespace GEngine;

int main() {

  // register RenderPass & add Objects(todo)
  // CSingleton<CRenderSystem>()->RegisterRenderPass(std::make_shared<CSkyboxPass>("skybox_pass", 1));
  // CSingleton<CRenderSystem>()->RegisterRenderPass(std::make_shared<CIBLPass>("ibl_pass", 2));

  // Precomputed Atmospherical Scattering demo
  // CSingleton<CRenderSystem>()->RegisterRenderPass(std::make_shared<PrecomputedAtmospherePass>("PrecomputedAtmospherePass", 1));

  // LTC demo
  CSingleton<CRenderSystem>()->RegisterRenderPass(std::make_shared<LTCDemoPass>("LTCDemoPass", 1));

  // CSingleton<CRenderSystem>()->AddLight(Omilight1);
  // CSingleton<CRenderSystem>()->RegisterRenderObject(Object);
  
  CSingleton<CApp>()->Init();
  CSingleton<CApp>()->RunMainLoop();
  return 0;
}
