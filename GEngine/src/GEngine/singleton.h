#pragma once
#include <memory>

namespace GEngine {
template <typename T> class CSingleton {
public:
  static T *Get() {
    static T s; // initialization of static members are thread-safe after c++11
    return &s;
  }
  // this allow: Singleton<T>()->func();
  T *operator->() const { return Get(); }
};
} // namespace GEngine
