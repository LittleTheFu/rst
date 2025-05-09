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
    camera_.Front = Eigen::Vector3f(0.0f, 0.0f, 1.0f);
    camera_.updateCameraVectors();

    // 4. 初始化网格
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>("teapot.obj");
    sceneData_.objects.push_back(std::move(mesh));
}

void Scene::run()
{
    gBufferPass_->Render(sceneData_, camera_);
}