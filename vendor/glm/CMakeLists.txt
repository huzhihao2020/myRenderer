project(glm)

file(GLOB_RECURSE SOURCES "include/*.inl")
file(GLOB_RECURSE SOURCES "include/*.hpp")
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(glm STATIC ${SOURCES})

target_include_directories(glm PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(glm INTERFACE -DGLM_FORCE_CXX20 -DGLM_ENABLE_EXPERIMENTAL)
