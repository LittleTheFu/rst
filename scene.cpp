#include "scene.h"
#include <iostream>
#include <glad/glad.h>
#include "material.h"
#include "texture.h"

void Scene::init()
{
    // 1. 初始化场景数据
    sceneData_.screenWidth = 800;
    sceneData_.screenHeight = 600;

    // 2. 初始化 G-Buffer Pass
    gBufferPass_ = std::make_unique<GBufferPass>();
    gBufferPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    // 2. 初始化 Light Pass
    // lightPass_ = std::make_unique<LightPass>();
    // lightPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    // 3. 初始化屏幕 Pass
    screenPass_ = std::make_unique<ScreenPass>();
    screenPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    // 3. 初始化相机
    camera_.Position = Eigen::Vector3f(0.0f, 0.0f, 10.0f);
    camera_.Front = Eigen::Vector3f(0.0f, 0.0f, -1.0f);
    camera_.updateCameraVectors();

    // 4. 初始化网格
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>("teapot.obj");

    std::shared_ptr<Texture> albedoTexture = std::make_shared<Texture>("lena.png");
    std::shared_ptr<Texture> normalTexture = std::make_shared<Texture>("normal.tga");
    std::shared_ptr<Texture> roughnessTexture = std::make_shared<Texture>("roughness.tga");
    std::shared_ptr<Texture> metallicTexture = std::make_shared<Texture>("metallic.tga");
    std::shared_ptr<Texture> aoTexture = std::make_shared<Texture>("ao.tga");

    std::shared_ptr<Material> material = std::make_shared<Material>("teapot_mtrl");
    material->setAlbedoMap(albedoTexture);
    material->setNormalMap(normalTexture);
    material->setRoughnessMap(roughnessTexture);
    material->setMetallicMap(metallicTexture);
    material->setAmbientOcclusionMap(aoTexture);

    mesh->setMaterial(material);

    sceneData_.objects.push_back(std::move(mesh));

}

void Scene::run()
{
    gBufferPass_->Render(sceneData_, camera_);
    screenPass_->Render(gBufferPass_->getColorAttachment(0),
                        gBufferPass_->getColorAttachment(1),
                        gBufferPass_->getColorAttachment(2),
                        gBufferPass_->getColorAttachment(3),
                        gBufferPass_->getColorAttachment(4),
                        gBufferPass_->getColorAttachment(5));
}
