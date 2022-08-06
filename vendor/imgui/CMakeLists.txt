project(imgui)

file(GLOB_RECURSE SOURCES imgui_demo.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui.cpp 
                          imconfig.h imgui_internal.h imgui.h
                          imstb_rectpack.h imstb_textedit.h imstb_truetype.h
                          backends/imgui_impl_opengl3_loader.h backends/imgui_impl_opengl3.h backends/imgui_impl_opengl3.cpp
                          backends/imgui_impl_glfw.h backends/imgui_impl_glfw.cpp)
                           
add_library(imgui STATIC ${SOURCES})

target_include_directories(imgui PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/backends ${CMAKE_SOURCE_DIR}/vendor)
target_compile_options(imgui INTERFACE -DGLM_FORCE_CXX20 -DGLM_ENABLE_EXPERIMENTAL)
