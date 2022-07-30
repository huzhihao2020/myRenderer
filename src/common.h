#pragma once
#include <string>
#include <limits>

#include "log.h"

namespace GEngine {
  const float PI = 3.14159265358f;
  const float EPSILON = std::numeric_limits<float>::min();

  namespace WINDOW_CONFIG {
    extern int WINDOW_WIDTH;
    extern int WINDOW_HEIGHT;
    extern int VIEWPORT_LOWERLEFT_X;
    extern int VIEWPORT_LOWERLEFT_Y;
    extern int VIEWPORT_WIDTH;
    extern int VIEWPORT_HEIGHT;
    extern std::string WINDOW_TITLE;
    extern bool IS_CURSOR_DISABLED;
    extern bool IS_MACOS_WINDOW; 
  }
}
