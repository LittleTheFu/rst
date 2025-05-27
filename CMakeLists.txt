cmake_minimum_required(VERSION 3.10)
project(OpenGL_GLAD_SDL2)

set(CMAKE_CXX_STANDARD 17)

# 添加 SDL2 子目录
add_subdirectory(third/sdl)
add_subdirectory(third/assimp)
add_subdirectory(third/lodepng)
add_subdirectory(third/gli)

# 禁用 Eigen 自带的测试
set(BUILD_TESTING OFF CACHE BOOL "Disable Eigen tests")

# 添加 Eigen 子目录
add_subdirectory(third/eigen)

# 查找 OpenGL
find_package(OpenGL REQUIRED)

# 添加可执行文件
add_executable(OpenGL_GLAD_SDL2 
    main.cpp 
    third/glad/src/glad.c
    glObject.cpp
    window.cpp
    vertex.cpp
    frameBuffer.cpp
    renderPass.cpp
    gBufferpass.cpp
    texture.cpp
    uniformBuffer.cpp
    glResource.cpp
    lightPass.cpp
    textureCubeMap.cpp
    material.cpp
    mesh.cpp
    shadowPass.cpp
    lightPass.cpp
    stb_image_wrapper.cpp
    skyPass.cpp
    iblPass.cpp
    texture2D.cpp
    textureCubeMap.cpp
    screenPass.cpp
    rendererConfig.cpp
    shader.cpp
    camera.cpp
    scene.cpp)

# 给可执行文件加 include 目录
target_include_directories(OpenGL_GLAD_SDL2 PRIVATE
    ${CMAKE_SOURCE_DIR}/third/glad/include
    ${CMAKE_SOURCE_DIR}/third/sdl/include
    ${CMAKE_SOURCE_DIR}/third/SDL/include
    ${CMAKE_SOURCE_DIR}/third/eigen
    ${CMAKE_SOURCE_DIR}/third/lodepng/include
    ${CMAKE_SOURCE_DIR}/third/stb_image/include
    ${CMAKE_SOURCE_DIR}/third/assimp/include
    ${CMAKE_CURRENT_BINARY_DIR}/third/assimp/include
    ${CMAKE_SOURCE_DIR}/third/gli
)

# 链接库
target_link_libraries(OpenGL_GLAD_SDL2
    SDL2::SDL2  # 如果找不到，就换成 SDL2
    ${OPENGL_LIBRARIES}
    assimp
    gli
)

# MSVC 特别处理
if (MSVC)
    add_compile_options(/source-charset:utf-8)
endif()
