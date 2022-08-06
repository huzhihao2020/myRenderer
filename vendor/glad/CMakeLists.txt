project(glad)

file(GLOB_RECURSE SOURCES "include/*.h")
file(GLOB_RECURSE SOURCES "src/*.c")

add_library(glad STATIC ${SOURCES})

target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/include)