cmake_minimum_required(VERSION 3.10)
project(OpenGL_GLAD_SDL2)

set(CMAKE_CXX_STANDARD 11)

# 添加 SDL2 子目录
add_subdirectory(third/sdl)

# 查找 OpenGL
find_package(OpenGL REQUIRED)

# 添加可执行文件
add_executable(OpenGL_GLAD_SDL2 main.cpp third/glad/src/glad.c)

# 给可执行文件加 include 目录
target_include_directories(OpenGL_GLAD_SDL2 PRIVATE
    ${CMAKE_SOURCE_DIR}/third/glad/include
    ${CMAKE_SOURCE_DIR}/third/sdl/include
)

# 链接库
target_link_libraries(OpenGL_GLAD_SDL2
    SDL2::SDL2  # 如果找不到，就换成 SDL2
    ${OPENGL_LIBRARIES}
)

# MSVC 特别处理
if (MSVC)
    add_compile_options(/source-charset:utf-8)
endif()
