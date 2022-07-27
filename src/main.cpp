#include <iostream>

#include "app.h"

// for test

void TestScene() {

}

int main() {
  std::cout << "Hello!\n";
  GEngine::CSingleton<GEngine::CApp>()->Init();
  GEngine::CSingleton<GEngine::CApp>()->RunMainLoop();
  return 0;
}
