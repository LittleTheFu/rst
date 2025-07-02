cmake_minimum_required(VERSION 3.10)
project(rst)

# set(CMAKE_Fortran_COMPILER_WORKS FALSE)
set(CMAKE_Fortran_COMPILER "")

enable_language(C CXX)

set(CMAKE_CXX_STANDARD 17)

# if (MSVC)
#     add_compile_options("/source-charset:utf-8")
# endif()

add_compile_definitions(JPH_DEBUG_RENDERER)


add_subdirectory(third/sdl)
add_subdirectory(third/assimp)
add_subdirectory(third/lodepng)
add_subdirectory(third/gli)
add_subdirectory(third/imgui)
add_subdirectory(third/JoltPhysics/Build)

add_subdirectory(third/freetype)
add_library(Freetype::Freetype ALIAS freetype)

add_subdirectory(third/RmlUi)


set(BUILD_TESTING OFF CACHE BOOL "Disable Eigen tests")

add_subdirectory(third/eigen)

find_package(OpenGL REQUIRED)

set(GAME_UI_DIR "gameUI")
set(GAME_UI_SOURCE_FILES
    ${GAME_UI_DIR}/RmlUiOpenGLRenderer.cpp
    ${GAME_UI_DIR}/RmlUiSystemInterface.cpp
    ${GAME_UI_DIR}/RmlUiFileInterface.cpp
)

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

set(UI_DIR "debugUI")
set(UI_SOURCE_FILES
    ${UI_DIR}/uiSystem.cpp
)

set(PHYSICS_DIR "physics")
set(PHYSICS_SOURCE_FILES
    ${PHYSICS_DIR}/physicsSystem.cpp
    ${PHYSICS_DIR}/joltDebugRenderer.cpp
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
    # ${MATERIAL_DIR}/materialFactory.cpp
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
    ${COLLISION_DIR}/ray.cpp
    ${COLLISION_DIR}/objectPicker.cpp
)

set(MESH_DIR "mesh")
set(MESH_SOURCE_FILES
    ${MESH_DIR}/mesh.cpp
    ${MESH_DIR}/model.cpp
    ${MESH_DIR}/assimpUtils.cpp
    ${MESH_DIR}/animation.cpp
    ${MESH_DIR}/animator.cpp
    ${MESH_DIR}/bone.cpp
)

set(CAMERA_DIR "camera")
set(CAMERA_SOURCE_FILES
    ${CAMERA_DIR}/camera.cpp
)

set(MANAGER_DIR "manager")
set(MANAGER_SOURCE_FILES
    ${MANAGER_DIR}/textureManager.cpp
    ${MANAGER_DIR}/materialManager.cpp
    ${MANAGER_DIR}/shaderManager.cpp
)

set(INTERFACE_DIR "interface")
set(INTERFACE_SOURCE_FILES
)

set(RST_DEBUG_DIR "rstDebug")
set(RST_DEBUG_SOURCE_FILES
    ${RST_DEBUG_DIR}/debugBox.cpp
    ${RST_DEBUG_DIR}/debugRenderer.cpp
)

set(COMMAND_DIR "command")
set(COMMAND_CORE_SOURCE_FILES
    ${COMMAND_DIR}/command.cpp
    ${COMMAND_DIR}/pickObjectCommand.cpp
)

set(OGL_DIR "ogl")
set(OGL_SOURCE_FILES
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
    ${CAMERA_COMMAND_DIR}/rotateCameraLeftCommand.cpp
    ${CAMERA_COMMAND_DIR}/rotateCameraRightCommand.cpp
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
    shadowUtils.cpp
    shader.cpp
    ${CAMERA_SOURCE_FILES}
    ${MANAGER_SOURCE_FILES}
    ${PASS_SOURCE_FILES}
    ${PHYSICS_SOURCE_FILES}
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
    ${MESH_SOURCE_FILES}
    ${INTERFACE_SOURCE_FILES}
    ${RST_DEBUG_SOURCE_FILES}
    ${GAME_UI_SOURCE_FILES}
    ${OGL_SOURCE_FILES}
)

target_compile_options(rst PRIVATE /wd4819)



target_include_directories(rst PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/${CAMERA_DIR}
    ${CMAKE_SOURCE_DIR}/${MANAGER_DIR}
    ${CMAKE_SOURCE_DIR}/${PASS_DIR}
    ${CMAKE_SOURCE_DIR}/${PHYSICS_DIR}
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
    ${CMAKE_SOURCE_DIR}/${MESH_DIR}
    ${CMAKE_SOURCE_DIR}/${INTERFACE_DIR}
    ${CMAKE_SOURCE_DIR}/${RST_DEBUG_DIR}
    ${CMAKE_SOURCE_DIR}/${GAME_UI_DIR}
    ${CMAKE_SOURCE_DIR}/${OGL_DIR}

    ${CMAKE_SOURCE_DIR}/third/glad/include
    ${CMAKE_SOURCE_DIR}/third/sdl/include
    ${CMAKE_SOURCE_DIR}/third/eigen
    ${CMAKE_SOURCE_DIR}/third/lodepng/include
    ${CMAKE_SOURCE_DIR}/third/stb_image/include
    ${CMAKE_SOURCE_DIR}/third/assimp/include
    ${CMAKE_CURRENT_BINARY_DIR}/third/assimp/include
    ${CMAKE_SOURCE_DIR}/third/gli
    ${CMAKE_SOURCE_DIR}/third/RmlUi/Include

)

target_link_libraries(rst
    SDL2::SDL2
    ${OPENGL_LIBRARIES}
    assimp
    gli
    imgui
    Jolt
    RmlUi::RmlUi
)

add_custom_command(
    TARGET rst POST_BUILD
    COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/cp_res.ps1"
    COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/cp_shader.ps1"
    COMMENT "Running PowerShell scripts after build"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)