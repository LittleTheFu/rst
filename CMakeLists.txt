cmake_minimum_required(VERSION 3.10)
project(rst)

# set(CMAKE_Fortran_COMPILER_WORKS FALSE)
set(CMAKE_Fortran_COMPILER "")

enable_language(C CXX)

set(CMAKE_CXX_STANDARD 17)

# if (MSVC)
#     add_compile_options("/source-charset:utf-8")
# endif()


add_subdirectory(third/sdl)
add_subdirectory(third/assimp)
add_subdirectory(third/lodepng)
add_subdirectory(third/gli)
add_subdirectory(third/imgui)


set(BUILD_TESTING OFF CACHE BOOL "Disable Eigen tests")

add_subdirectory(third/eigen)

find_package(OpenGL REQUIRED)

set(BUFFER_DIR "buffer")
set(BUFFER_SOURCE_FILES
    ${BUFFER_DIR}/vertexBuffer.cpp
    ${BUFFER_DIR}/indexBuffer.cpp
    ${BUFFER_DIR}/vertexArray.cpp
    ${BUFFER_DIR}/frameBuffer.cpp
    ${BUFFER_DIR}/uniformBuffer.cpp
)

set(TEXTURE_DIR "texture")
set(TEXTURE_SOURCE_FILES
    ${TEXTURE_DIR}/texture.cpp
    ${TEXTURE_DIR}/texture2D.cpp
    ${TEXTURE_DIR}/textureCubeMap.cpp
)

set(UI_DIR "ui")
set(UI_SOURCE_FILES
    ${UI_DIR}/uiSystem.cpp
)

set(SCENE_DIR "scene")
set(SCENE_SOURCE_FILES
    ${SCENE_DIR}/scene.cpp
    ${SCENE_DIR}/sceneData.cpp
    ${SCENE_DIR}/uiSceneData.cpp
)

set(UTILITIES_DIR "utilities")
set(UTILITIES_SOURCE_FILES
    ${UTILITIES_DIR}/utilities.cpp
    ${UTILITIES_DIR}/stb_image_wrapper.cpp
)
set(MATERIAL_DIR "material")
set(MATERIAL_SOURCE_FILES
    ${MATERIAL_DIR}/material.cpp
    ${MATERIAL_DIR}/materialFactory.cpp
)

set(PASS_DIR "pass")
set(PASS_SOURCE_FILES
    ${PASS_DIR}/renderPass.cpp
    ${PASS_DIR}/gBufferpass.cpp
    ${PASS_DIR}/lightPass.cpp
    ${PASS_DIR}/screenPass.cpp
    ${PASS_DIR}/oitPass.cpp
    ${PASS_DIR}/depthOfFieldPass.cpp
    ${PASS_DIR}/blurHorizontalPass.cpp
    ${PASS_DIR}/blurVerticalPass.cpp
    ${PASS_DIR}/shadowPass.cpp
    ${PASS_DIR}/ssrPass.cpp
    ${PASS_DIR}/skyPass.cpp
    ${PASS_DIR}/iblPass.cpp
    ${PASS_DIR}/combinedPass.cpp
    # ${PASS_DIR}/ssaoPass.cpp
    ${PASS_DIR}/postPass.cpp
    ${PASS_DIR}/brightnessMaskPass.cpp
    ${PASS_DIR}/godRayPass.cpp
)

set(LIGHT_DIR "light")
set(LIGHT_SOURCE_FILES
)

set(INPUT_DIR "input")
set(INPUT_SOURCE_FILES
    ${INPUT_DIR}/inputManager.cpp
)

set(COLLISION_DIR "collision")
set(COLLISION_SOURCE_FILES
    ${COLLISION_DIR}/boundingVolume.cpp
)

set(RST_DEBUG_DIR "rstDebug")
set(RST_DEBUG_SOURCE_FILES
    ${RST_DEBUG_DIR}/debugBox.cpp
)

set(COMMAND_DIR "command")
set(COMMAND_CORE_SOURCE_FILES
    ${COMMAND_DIR}/command.cpp
)

set(CAMERA_COMMAND_DIR "${COMMAND_DIR}/cameraCommand")
set(CAMERA_COMMAND_SOURCE_FILES
    ${CAMERA_COMMAND_DIR}/cameraCommand.cpp
    ${CAMERA_COMMAND_DIR}/moveCameraBackwardCommand.cpp
    ${CAMERA_COMMAND_DIR}/moveCameraDownCommand.cpp
    ${CAMERA_COMMAND_DIR}/moveCameraForwardCommand.cpp
    ${CAMERA_COMMAND_DIR}/moveCameraLeftCommand.cpp
    ${CAMERA_COMMAND_DIR}/moveCameraRightCommand.cpp
    ${CAMERA_COMMAND_DIR}/moveCameraUpCommand.cpp
    ${CAMERA_COMMAND_DIR}/processMouseMovementCommand.cpp
    ${CAMERA_COMMAND_DIR}/processMouseScrollCommand.cpp
)

set(DEBUG_COMMAND_DIR "${COMMAND_DIR}/debugCommand")
set(DEBUG_COMMAND_SOURCE_FILES
    ${DEBUG_COMMAND_DIR}/toggleDebugModeCommand.cpp
)

set(COMMAND_SOURCE_FILES
    ${COMMAND_CORE_SOURCE_FILES}
    ${CAMERA_COMMAND_SOURCE_FILES}
    ${DEBUG_COMMAND_SOURCE_FILES}
)

add_executable(rst
    main.cpp
    third/glad/src/glad.c
    skyBoxCube.cpp
    window.cpp
    vertex.cpp
    screenQuad.cpp
    glResource.cpp
    mesh.cpp
    shadowUtils.cpp
    shader.cpp
    camera.cpp
    ${PASS_SOURCE_FILES}
    ${UTILITIES_SOURCE_FILES}
    ${SCENE_SOURCE_FILES}
    ${UI_SOURCE_FILES}
    ${TEXTURE_SOURCE_FILES}
    ${BUFFER_SOURCE_FILES}
    ${MATERIAL_SOURCE_FILES}
    ${LIGHT_SOURCE_FILES}
    ${INPUT_SOURCE_FILES}
    ${COMMAND_SOURCE_FILES}
    ${COLLISION_SOURCE_FILES}
    ${RST_DEBUG_SOURCE_FILES}
)

target_compile_options(rst PRIVATE /wd4819)


target_include_directories(rst PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/${PASS_DIR}
    ${CMAKE_SOURCE_DIR}/${UTILITIES_DIR}
    ${CMAKE_SOURCE_DIR}/${SCENE_DIR}
    ${CMAKE_SOURCE_DIR}/${UI_DIR}
    ${CMAKE_SOURCE_DIR}/${TEXTURE_DIR}
    ${CMAKE_SOURCE_DIR}/${BUFFER_DIR}
    ${CMAKE_SOURCE_DIR}/${MATERIAL_DIR}
    ${CMAKE_SOURCE_DIR}/${LIGHT_DIR}
    ${CMAKE_SOURCE_DIR}/${INPUT_DIR}
    ${CMAKE_SOURCE_DIR}/${COMMAND_DIR}
    ${CMAKE_SOURCE_DIR}/${CAMERA_COMMAND_DIR}
    ${CMAKE_SOURCE_DIR}/${DEBUG_COMMAND_DIR}
    ${CMAKE_SOURCE_DIR}/${COLLISION_DIR}
    ${CMAKE_SOURCE_DIR}/${RST_DEBUG_DIR}


    ${CMAKE_SOURCE_DIR}/third/glad/include
    ${CMAKE_SOURCE_DIR}/third/sdl/include
    ${CMAKE_SOURCE_DIR}/third/eigen
    ${CMAKE_SOURCE_DIR}/third/lodepng/include
    ${CMAKE_SOURCE_DIR}/third/stb_image/include
    ${CMAKE_SOURCE_DIR}/third/assimp/include
    ${CMAKE_CURRENT_BINARY_DIR}/third/assimp/include
    ${CMAKE_SOURCE_DIR}/third/gli
)

target_link_libraries(rst
    SDL2::SDL2
    ${OPENGL_LIBRARIES}
    assimp
    gli
    imgui
)

add_custom_command(
    TARGET rst POST_BUILD
    COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/cp_res.ps1"
    COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/cp_shader.ps1"
    COMMENT "Running PowerShell scripts after build"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)