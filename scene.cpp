#include "scene.h"
#include <iostream>
#include <glad/glad.h>
#include "material.h"
#include "texture.h"
#include "pointLight.h"
#include "cubeMap.h"

void Scene::init()
{
    // 1. 初始化场景数据
    sceneData_.screenWidth = 800;
    sceneData_.screenHeight = 600;

    // 2. 初始化 G-Buffer Pass
    gBufferPass_ = std::make_unique<GBufferPass>();
    gBufferPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    lightPass_ = std::make_unique<LightPass>();
    lightPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    skyPass_ = std::make_unique<SkyPass>();
    skyPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    screenPass_ = std::make_unique<ScreenPass>();
    screenPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    // 3. 初始化相机
    camera_.Position = Eigen::Vector3f(0.0f, 0.0f, 3.0f);
    camera_.Front = Eigen::Vector3f(0.0f, 0.0f, -1.0f);
    // camera_.Front = -camera_.Position;
    camera_.updateCameraVectors();

    // 4. 初始化网格
    std::unique_ptr<Mesh> mesh_teapot = std::make_unique<Mesh>("teapot.obj");
    std::unique_ptr<Mesh> mesh_box = std::make_unique<Mesh>("bx.obj");

    std::shared_ptr<Texture> albedoTexture = std::make_shared<Texture>("lena.png");
    // std::shared_ptr<Texture> albedoTexture = std::make_shared<Texture>("color.tga");
    std::shared_ptr<Texture> normalTexture = std::make_shared<Texture>("normal.tga");
    std::shared_ptr<Texture> roughnessTexture = std::make_shared<Texture>("roughness.tga");
    std::shared_ptr<Texture> metallicTexture = std::make_shared<Texture>("metallic.tga");
    std::shared_ptr<Texture> aoTexture = std::make_shared<Texture>("ao.tga");

    std::shared_ptr<Material> material_teapot = std::make_shared<Material>("teapot_mtrl");
    material_teapot->setAlbedoMap(albedoTexture);
    material_teapot->setNormalMap(normalTexture);
    material_teapot->setRoughnessMap(roughnessTexture);
    material_teapot->setMetallicMap(metallicTexture);
    material_teapot->setAmbientOcclusionMap(aoTexture);

    // std::shared_ptr<Cubemap> cubemap = std::make_shared<Cubemap>({"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"});
    std::shared_ptr<Cubemap> cubemapPtr = std::make_shared<Cubemap>(std::vector<std::string>{
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"});
    std::shared_ptr<Material> material_cubemap = std::make_shared<Material>("cubemap_mtrl");
    material_cubemap->setCubemap(cubemapPtr);
    // sceneData_.skybox = cubemapPtr;

    mesh_teapot->setMaterial(material_teapot);
    mesh_box->setMaterial(material_cubemap);

    float teapot_scale = 1.0f;
    mesh_teapot->setScale(Eigen::Vector3f(teapot_scale, teapot_scale, teapot_scale));

    float box_scale = 30.0f;
    mesh_box->setScale(Eigen::Vector3f(box_scale, box_scale, box_scale));

    mesh_teapot->setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    sceneData_.objects.push_back(std::move(mesh_teapot));
    // sceneData_.objects.push_back(std::move(mesh_box));
    sceneData_.skybox = std::move(mesh_box);

    // 5. 初始化光源
    sceneData_.light = std::make_shared<PointLight>();
    sceneData_.light->position = Eigen::Vector3f(0.0f, 0.0f, -30.0f);
    sceneData_.light->color = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    sceneData_.light->intensity = 10.0f;
}

void Scene::run()
{
    //for debug,test light---------------------------------------------
    static int count = 0;
    count++;
    count %= 48000;
    float x = count / 1200.0f - 20.0f;
    x *= 2;
    sceneData_.light->position = Eigen::Vector3f(x, 0, -5.0f);
    sceneData_.light->intensity = 90.0f;

    // float scale = (count % 10000) / 100.0f;
    // scale /= 100.0f;
    // sceneData_.skybox->setScale(Eigen::Vector3f(scale, scale, scale));

    // float scale = (count % 10000) / 100.0f;
    // sceneData_.objects.at(0)->setScale(Eigen::Vector3f(scale, scale, scale));
    // -----------------------------------------------------------------

    glDisable(GL_CULL_FACE);
    gBufferPass_->Render(sceneData_, camera_);
    lightPass_->Render(gBufferPass_->getColorAttachment(0),
                        gBufferPass_->getColorAttachment(1),
                        gBufferPass_->getColorAttachment(2),
                        gBufferPass_->getColorAttachment(3),
                        gBufferPass_->getColorAttachment(4),
                        gBufferPass_->getColorAttachment(5),
                        sceneData_.light,
                        camera_);
    skyPass_->Render(sceneData_, camera_);
    screenPass_->Render(lightPass_->getColorAttachment(0),
                        skyPass_->getColorTexture(),
                        gBufferPass_->getDepthAttachment());
    // screenPass_->Render(lightPass_->getColorAttachment(0));
}