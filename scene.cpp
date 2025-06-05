#include "scene.h"
#include <iostream>
#include <glad/glad.h>
#include "debug_utils.h"
#include "materialFactory.h"

void Scene::init()
{
    // 1. 初始化场景数据 (屏幕/阴影尺寸等，可以根据需要精简SceneData)
    sceneData_.screenWidth = 800;
    sceneData_.screenHeight = 600;
    sceneData_.shadowMapWidth = 1024;
    sceneData_.shadowMapHeight = 1024;

    // 2. 初始化相机
    camera_.Position = Eigen::Vector3f(0.0f, 2.0f, 18.0f);
    camera_.lookAt(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    camera_.updateCameraVectors();

    // 3. 初始化主光源
    mainLight_ = std::make_shared<PointLight>();
    mainLight_->position = Eigen::Vector3f(0.0f, 0.0f, -30.0f);
    mainLight_->color = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    mainLight_->intensity = 8.0f;

    // 5. 加载 IBL 纹理 (在创建 IBLPass 和 SkyPass 之前加载)
    std::shared_ptr<TextureCubeMap> irradianceMapTex_ = TextureCubeMap::loadDDS("ibl/house/houseDiffuseHDR.dds");
    if (!irradianceMapTex_) {
        std::cerr << "ERROR::SCENE::Failed to load irradiance map! Check path and DDS format." << std::endl;
    }

    std::shared_ptr<TextureCubeMap> prefilterMapTex_ = TextureCubeMap::loadDDS("ibl/house/houseSpecularHDR.dds");
    if (!prefilterMapTex_) {
        std::cerr << "ERROR::SCENE::Failed to load prefilter map! Check path and DDS format." << std::endl;
    }

    std::shared_ptr<Texture2D> brdfLUTTex_ = Texture2D::loadDDS("ibl/house/houseBrdf.dds");
    if (!brdfLUTTex_) {
        std::cerr << "ERROR::SCENE::Failed to load BRDF LUT! Check path and DDS format." << std::endl;
    }

    // 6. 初始化渲染 Pass (构造函数现在更简洁，只负责Pass自身的FBO等初始化)
    skyPass_ = std::make_unique<SkyPass>(sceneData_.screenWidth, sceneData_.screenHeight, prefilterMapTex_);
    gBufferPass_ = std::make_unique<GBufferPass>(sceneData_.screenWidth, sceneData_.screenHeight);
    shadowPass_ = std::make_unique<ShadowPass>(sceneData_.shadowMapWidth, sceneData_.shadowMapHeight);
    lightPass_ = std::make_unique<LightPass>(sceneData_.screenWidth, sceneData_.screenHeight);

    oitPass_ = std::make_unique<OitPass>(
        sceneData_.screenWidth,
        sceneData_.screenHeight,
        irradianceMapTex_,
        prefilterMapTex_,
        brdfLUTTex_);

    // IBLPass 和 SkyPass 的构造函数仍然可以注入其不变的IBL纹理
    iblPass_ = std::make_unique<IBLPass>(
        sceneData_.screenWidth,
        sceneData_.screenHeight,
        irradianceMapTex_,
        prefilterMapTex_,
        brdfLUTTex_);

    combinedPass_ = std::make_unique<CombinedPass>(sceneData_.screenWidth, sceneData_.screenHeight);
    blurHorizontalPass_ = std::make_unique<BlurHorizontalPass>(sceneData_.screenWidth, sceneData_.screenHeight);
    blurVerticalPass_ = std::make_unique<BlurVerticalPass>(sceneData_.screenWidth, sceneData_.screenHeight);
    depthOfFieldPass_ = std::make_unique<DepthOfFieldPass>(sceneData_.screenWidth, sceneData_.screenHeight);

    postPass_ = std::make_unique<PostPass>(sceneData_.screenWidth, sceneData_.screenHeight);
    screenPass_ = std::make_unique<ScreenPass>(sceneData_.screenWidth, sceneData_.screenHeight);

    //---create materials-----
    std::shared_ptr<Material> goldMaterial = MaterialFactory::CreateMaterialFromDirectory("goldMaterial", "gold");
    std::shared_ptr<Material> plasticMaterial = MaterialFactory::CreateMaterialFromDirectory("plasticMaterial", "plastic");
    std::shared_ptr<Material> rustedIronMaterial = MaterialFactory::CreateMaterialFromDirectory("rustedIronMaterial", "rusted_iron");
    std::shared_ptr<Material> grassMaterial = MaterialFactory::CreateMaterialFromDirectory("grassMaterial", "grass");
    std::shared_ptr<Material> wallMaterial = MaterialFactory::CreateMaterialFromDirectory("wallMaterial", "wall");
    std::shared_ptr<Material> silverMaterial = MaterialFactory::CreateMaterialFromDirectory("silverMaterial", "silver");
    
    //---gun--------------------------------------------------------------------------------------------------
    std::shared_ptr<Texture2D> gunAlbedoTexture = std::move(Texture2D::loadFromFile("gun/Textures/Cerberus_A.tga", false, true));
    std::shared_ptr<Texture2D> gunNormalTexture = std::move(Texture2D::loadFromFile("gun/Textures/Cerberus_N.tga"));
    std::shared_ptr<Texture2D> gunRoughnessTexture = std::move(Texture2D::loadFromFile("gun/Textures/Cerberus_R.tga"));
    std::shared_ptr<Texture2D> gunMetallicTexture = std::move(Texture2D::loadFromFile("gun/Textures/Cerberus_M.tga"));
    std::shared_ptr<Texture2D> gunAoTexture = std::move(Texture2D::loadFromFile("gun/Textures/raw/Cerberus_AO.tga"));

    std::shared_ptr<Material> gunMaterial = std::make_shared<Material>("gunMaterial");
    gunMaterial->setAlbedoMap(gunAlbedoTexture);
    gunMaterial->setNormalMap(gunNormalTexture);
    gunMaterial->setRoughnessMap(gunRoughnessTexture);
    gunMaterial->setMetallicMap(gunMetallicTexture);
    gunMaterial->setAmbientOcclusionMap(gunAoTexture);

    // float scale = 0.06f; // 调整缩放比例
    // std::unique_ptr<Mesh> mesh_gun = std::make_unique<Mesh>("gun/Cerberus_LP.FBX");
    // mesh_gun->setMaterial(gunMaterial);
    // mesh_gun->setRotation(Eigen::Quaternionf(M_PI/4, 0.0f, 1.0f, 0.0f));
    // mesh_gun->setScale(Eigen::Vector3f(scale, scale, scale));
    // mesh_gun->setPosition(Eigen::Vector3f(0.0f, 5.0f, 0.0f));
    // sceneData_.opaqueObjects.push_back(std::move(mesh_gun));

    //----------------------------------------------------------------------------------------

    // 创建网格并设置材质和变换
    std::unique_ptr<Mesh> mesh_teapot = std::make_unique<Mesh>("teapot.obj");
    mesh_teapot->setMaterial(silverMaterial);
    mesh_teapot->setScale(Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    mesh_teapot->setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    sceneData_.opaqueObjects.push_back(std::move(mesh_teapot));
    // sceneData_.transparentObjects.push_back(std::move(mesh_teapot));

    // std::unique_ptr<Mesh> mesh_box = std::make_unique<Mesh>("bx.obj");
    // mesh_box->setMaterial(wallMaterial);
    // mesh_box->setPosition(Eigen::Vector3f(0.0f, 0.0f, -12.0f));
    // mesh_box->setScale(Eigen::Vector3f(10.0f, 10.0f, 10.0f));
    // sceneData_.opaqueObjects.push_back(std::move(mesh_box));

    std::unique_ptr<Mesh> mesh_transparent_teapot = std::make_unique<Mesh>("teapot.obj");
    mesh_transparent_teapot->setMaterial(grassMaterial);
    mesh_transparent_teapot->setScale(Eigen::Vector3f(5, 5, 5));
    mesh_transparent_teapot->setPosition(Eigen::Vector3f(0.0f, 0.0f, -30.0f));
    sceneData_.opaqueObjects.push_back(std::move(mesh_transparent_teapot));
    // sceneData_.transparentObjects.push_back(std::move(mesh_transparent_teapot));

    std::unique_ptr<Mesh> mesh_cursor = std::make_unique<Mesh>("bx.obj");
    mesh_cursor->setMaterial(goldMaterial);
    mesh_cursor->setScale(Eigen::Vector3f(0.2f, 0.2f, 0.2f));
    sceneData_.opaqueObjects.push_back(std::move(mesh_cursor));
}

void Scene::run()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // 背面剔除

    // 光源调试动画
    static int count = 0;
    count++;
    count %= 24000;
    float x_light = count / 1200.0f - 10.0f;
    x_light *= 0.5;
    mainLight_->position = Eigen::Vector3f(x_light, x_light, 7.0f);
    // mainLight_->position = Eigen::Vector3f(5, 5, 7.0f);
    mainLight_->intensity = 15.0f;

    // 调试光标位置
    Eigen::Vector3f offset = Eigen::Vector3f(0.0f, 0.5f, 0.0f);
    if(!sceneData_.opaqueObjects.empty())
    {
        sceneData_.opaqueObjects.back()->setPosition(mainLight_->position + offset);
    }

    // --- 渲染管线执行 ---

    skyPass_->Render(camera_);
    GL_CHECK_ERROR();


    shadowPass_->Render(sceneData_.opaqueObjects, *mainLight_);
    GL_CHECK_ERROR();

    gBufferPass_->Render(sceneData_.opaqueObjects, camera_);
    GL_CHECK_ERROR();

    oitPass_->Render(sceneData_.transparentObjects,
                     *mainLight_,
                     camera_,
                     gBufferPass_->getDepthTextureId());
    GL_CHECK_ERROR();

    lightPass_->Render(gBufferPass_->getPositionTextureId(), // gPosition
                       gBufferPass_->getNormalTextureId(), // gNormal
                       gBufferPass_->getAlbedoTextureId(), // gAlbedo
                       gBufferPass_->getRoughnessTextureId(), // gRoughness
                       gBufferPass_->getMetallicTextureId(), // gMetallic
                       gBufferPass_->getAOTextureId(), // gAO
                       *mainLight_,
                       camera_,
                       shadowPass_->getShadowMapDepthOutputTextureId());
    GL_CHECK_ERROR();

    iblPass_->Render(gBufferPass_->getPositionTextureId(), // gPosition
                     gBufferPass_->getNormalTextureId(), // gNormal
                     gBufferPass_->getAlbedoTextureId(), // gAlbedo
                     gBufferPass_->getRoughnessTextureId(), // gRoughness
                     gBufferPass_->getMetallicTextureId(), // gMetallic
                     gBufferPass_->getAOTextureId(), // gAO
                     camera_);
    GL_CHECK_ERROR();

    combinedPass_->Render(lightPass_->getOutputTextureId(),
                        iblPass_->getColorTextureId(),
                        gBufferPass_->getDepthTextureId(),
                        oitPass_->getAccumTextureId(),
                        oitPass_->getRevealTextureId(),
                        skyPass_->getColorTextureId());
    GL_CHECK_ERROR();

    blurHorizontalPass_->Render(combinedPass_->getColorTextureId());
    GL_CHECK_ERROR();

    blurVerticalPass_->Render(blurHorizontalPass_->getColorTextureId());
    GL_CHECK_ERROR();

    depthOfFieldPass_->Render(combinedPass_->getColorTextureId(),
                              blurVerticalPass_->getColorTextureId(),
                              gBufferPass_->getDepthTextureId(),
                              18.0f,
                              10.0f,
                              camera_.nearClip,
                              camera_.farClip);

    postPass_->Render(depthOfFieldPass_->getColorTextureId());

    // postPass_->Render(combinedPass_->getColorTextureId());
    GL_CHECK_ERROR();

    screenPass_->Render(postPass_->getColorTextureId());
    GL_CHECK_ERROR();
}

void Scene::resize(int width, int height)
{
    // 更新 SceneData 的尺寸
    sceneData_.screenWidth = width;
    sceneData_.screenHeight = height;

    // 逐个调用所有 Pass 的 resize 方法
    if (gBufferPass_)
    {
        gBufferPass_->Resize(width, height);
    }

    if (lightPass_)
    {
        lightPass_->Resize(width, height);
    }

    if (skyPass_)
    {
        skyPass_->Resize(width, height);
    }

    if (combinedPass_)
    {
        combinedPass_->Resize(width, height);
    }

    if (blurHorizontalPass_)
    {
        blurHorizontalPass_->Resize(width, height);
    }

    if (blurVerticalPass_)
    {
        blurVerticalPass_->Resize(width, height);
    }

    if (depthOfFieldPass_)
    {
        depthOfFieldPass_->Resize(width, height);
    }
    
    if (iblPass_)
    {
        iblPass_->Resize(width, height);
    }

    if (postPass_)
    {
        postPass_->Resize(width, height);
    }

    if (screenPass_)
    {
        screenPass_->Resize(width, height);
    }

    if (oitPass_)
    {
        oitPass_->Resize(width, height);
    }

    // 更新主相机的投影矩阵，以适应新的屏幕宽高比
    camera_.setAspectRatio(static_cast<float>(width) / height);
    // camera_.updateProjectionMatrix();
}