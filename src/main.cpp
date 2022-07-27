#include <iostream>

#include "app.h"

// for test

int main() {

  std::cout << "hello!\n";
  GEngine::CApp::Instance().Init();
  GEngine::CApp::Instance().RunMainLoop();
  return 0;
}
