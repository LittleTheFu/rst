#include "scene.h"
#include <iostream>
#include <glad/glad.h>

void Scene::init()
{
    // 1. 初始化场景数据
    sceneData_.screenWidth = 800;
    sceneData_.screenHeight = 600;

    // 2. 初始化 G-Buffer Pass
    gBufferPass_ = std::make_unique<GBufferPass>();
    gBufferPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    // 3. 初始化相机
    camera_.Position = Eigen::Vector3f(0.0f, 0.0f, 5.0f);
    camera_.updateCameraVectors();


    // ... (加载和编译着色器的代码)
    // vertex_shader_ = ...;
    // fragment_shader_ = ...;
    // shaderProgram_ = createShaderProgram(vertex_shader_, fragment_shader_);

    // ... (创建 VAO 和 VBO 的代码)
    // glGenVertexArrays(1, &VAO_);
    // glGenBuffers(1, &VBO_);
    // ...
}

void Scene::run()
{
    gBufferPass_->Render(sceneData_, camera_);
}