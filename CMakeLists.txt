cmake_minimum_required(VERSION 3.22.1)
project(MyRenderLab VERSION 0.1 DESCRIPTION "MyRenderLab" LANGUAGES CXX) 

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(CMAKE_XCODE_ATTRIBUTE_CXX_CLANG_LANGUAGE_STANFARD "-std=c++20")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0.1")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  error("Linux not supported yet")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  error("Windows not supported yet")
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/assimp)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glad)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/GLFW)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/stb)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/imgui)

add_subdirectory(${PROJECT_SOURCE_DIR}/GEngine)
add_subdirectory(${PROJECT_SOURCE_DIR}/Sandbox)

file(GLOB_RECURSE SOURCES
  "${GEngine_SOURCE_DIR}/*.h"
  "${GEngine_SOURCE_DIR}/*.cpp"
)

add_library(myRenderer SHARED ${SOURCES})
target_link_libraries(myRenderer PUBLIC 
  glm
  glad
  imgui
  ${vendor_SOURCE_DIR}/lib/libglfw.3.dylib
  ${vendor_SOURCE_DIR}/lib/libassimp.5.2.0.dylib
  "-framework OpenGL"
  "-framework Cocoa" 
  "-framework IOKit"
)

target_include_directories(myRenderer PUBLIC ${vendor_SOURCE_DIR})
target_include_directories(myRenderer PRIVATE ${GEngine_SOURCE_DIR}/src ${GEngine_SOURCE_DIR}/src/GEngine)

file(GLOB_RECURSE SANDBOX_SOURCES
  "${Sandbox_SOURCE_DIR}/*.h"
  "${Sandbox_SOURCE_DIR}/*.cpp"
)

add_executable(Application ${SANDBOX_SOURCES})
target_include_directories(Application PRIVATE ${GEngine_SOURCE_DIR} ${GEngine_SOURCE_DIR}/src ${GEngine_SOURCE_DIR}/src/GEngine)
target_link_libraries(Application PRIVATE myRenderer)