imguiFiles = {
  "imgui/imconfig.h",
  "imgui/imgui_internal.h",
  "imgui/imgui.cpp",
  "imgui/imgui.h",
  "imgui/imgui_widgets.cpp",
  "imgui/imgui_draw.cpp",
  "imgui/imgui_tables.cpp",
  "imgui/backends/imgui_impl_glfw.h",
  "imgui/backends/imgui_impl_glfw.cpp",
  "imgui/backends/imgui_impl_opengl3.h",
  "imgui/backends/imgui_impl_opengl3.cpp",
}

workspace "MyRenderLab"
  configurations { "Debug", "Release" }
  location "build"
  platforms {"Mac_Intel"}

project "MyRenderer"
  kind "ConsoleApp"
  --kind "SharedLib"
  systemversion "12.0"
  language "C++"
  targetdir "bin/%{cfg.buildcfg}"
  cppdialect "C++20"
  
  files {imguiFiles}
  files { "src/**.h", "src/**.cpp", "src/**.hpp"}
  files { "vendor/**.h", "vendor/**.cpp", "vendor/**.hpp", "vendor/**.c"}

  -- system include paths
  sysincludedirs {
    "vendor",
    "vendor/glad/include",
    "vendor/GLFW",
    "vendor/imgui",
    "vendor/glm",
    "vendor/stb",
    "vendor/assimp",
  }
  -- include paths
  includedirs {
    "vendor/imgui",
    "vendor/imgui/backends",
  }
  -- lib paths
  libdirs {
    "vendor/lib"
  }
  -- libs and frameworks
  links {"glfw3", "OpenGL.framework", "Cocoa.framework", "IOKit.framework"}

  filter "configurations:Debug"
    defines{"DEBUG"}
    symbols "On"
  
  filter "configurations:Release"
    defines{"NDEBUG"}
    optimize "On"



  libdirs {"vendor/lib"}
