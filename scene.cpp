#include "scene.h"
#include <iostream>
#include <glad/glad.h>
#include "debug_utils.h" // 确保包含调试工具

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

    // 4. 初始化阴影相机 (点光源阴影通常需要6个面，这里只设置基本属性)
    shadow_camera_.setAspectRatio(1.0f); // 阴影贴图通常是正方形
    shadow_camera_.setFOV(90.0f);        // 90度FOV用于点光源的立方体阴影贴图

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
    gBufferPass_ = std::make_unique<GBufferPass>(sceneData_.screenWidth, sceneData_.screenHeight);
    shadowPass_ = std::make_unique<ShadowPass>(sceneData_.shadowMapWidth, sceneData_.shadowMapHeight);
    lightPass_ = std::make_unique<LightPass>(sceneData_.screenWidth, sceneData_.screenHeight);
    oitPass_ = std::make_unique<OitPass>(sceneData_.screenWidth, sceneData_.screenHeight);

    // IBLPass 和 SkyPass 的构造函数仍然可以注入其不变的IBL纹理
    iblPass_ = std::make_unique<IBLPass>(
        sceneData_.screenWidth, sceneData_.screenHeight,
        irradianceMapTex_, prefilterMapTex_, brdfLUTTex_);

    // skyPass_ = std::make_unique<SkyPass>(
    //     sceneData_.screenWidth, sceneData_.screenHeight,
    //     irradianceMapTex_); // 或 prefilterMapTex_，取决于天空盒材质的真实性需求

    screenPass_ = std::make_unique<ScreenPass>(sceneData_.screenWidth, sceneData_.screenHeight);

    // auto test = Texture2D::loadFromFile("gold/albedo.png");
    // 7. 初始化网格和材质
    // 加载LDR纹理
    const std::string folder = "wall/";
    // const std::string folder = "rusted_iron/";
    // const std::string folder = "gold/";
    std::shared_ptr<Texture2D> albedoTexture = std::move(Texture2D::loadFromFile(folder + "albedo.png"));
    std::shared_ptr<Texture2D> normalTexture = std::move(Texture2D::loadFromFile(folder + "normal.png"));
    std::shared_ptr<Texture2D> roughnessTexture = std::move(Texture2D::loadFromFile(folder + "roughness.png"));
    std::shared_ptr<Texture2D> metallicTexture = std::move(Texture2D::loadFromFile(folder + "metallic.png"));
    std::shared_ptr<Texture2D> aoTexture = std::move(Texture2D::loadFromFile(folder + "ao.png"));

    // 创建材质
    std::shared_ptr<Material> material_teapot = std::make_shared<Material>("teapot_mtrl");
    material_teapot->setAlbedoMap(albedoTexture);
    material_teapot->setNormalMap(normalTexture);
    material_teapot->setRoughnessMap(roughnessTexture);
    material_teapot->setMetallicMap(metallicTexture);
    material_teapot->setAmbientOcclusionMap(aoTexture);

    // 创建网格并设置材质和变换
    std::unique_ptr<Mesh> mesh_teapot = std::make_unique<Mesh>("teapot.obj");
    mesh_teapot->setMaterial(material_teapot);
    mesh_teapot->setScale(Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    mesh_teapot->setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    meshes_.push_back(std::move(mesh_teapot));

    std::unique_ptr<Mesh> mesh_box = std::make_unique<Mesh>("bx.obj");
    mesh_box->setMaterial(material_teapot);
    mesh_box->setPosition(Eigen::Vector3f(0.0f, 0.0f, -12.0f));
    mesh_box->setScale(Eigen::Vector3f(10.0f, 10.0f, 10.0f));
    meshes_.push_back(std::move(mesh_box));

    std::unique_ptr<Mesh> mesh_cursor = std::make_unique<Mesh>("bx.obj");
    mesh_cursor->setMaterial(material_teapot);
    mesh_cursor->setScale(Eigen::Vector3f(0.2f, 0.2f, 0.2f));
    meshes_.push_back(std::move(mesh_cursor));
}

void Scene::run()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // 背面剔除

    // 光源调试动画
    static int count = 0;
    count++;
    count %= 48000;
    float x_light = count / 1200.0f - 15.0f;
    x_light *= 0.5;
    mainLight_->position = Eigen::Vector3f(x_light, x_light, 7.0f);
    // mainLight_->position = Eigen::Vector3f(5, 5, 7.0f);
    mainLight_->intensity = 100.0f;

    // 调试光标位置
    Eigen::Vector3f offset = Eigen::Vector3f(0.0f, 0.5f, 0.0f);
    if(!meshes_.empty())
    {
        meshes_.back()->setPosition(mainLight_->position + offset);
    }

    // 设置阴影相机 (更新位置)
    // 对于点光源，shadow_camera_ 的 GetLightSpaceMatrices() 方法
    // 会根据 mainLight_->position 和 far_plane 生成6个视图-投影矩阵。
    shadow_camera_.Position = mainLight_->position;

    // 获取需要用于Pass的网格列表 (转换为const Mesh*，避免所有权问题)
    std::vector<const Mesh*> rawMeshes;
    for (const auto& mesh : meshes_) {
        rawMeshes.push_back(mesh.get());
    }

    // --- 渲染管线执行 ---

    // 1. 渲染阴影贴图 (Shadow Pass)
    shadowPass_->Render(rawMeshes, *mainLight_);
    GL_CHECK_ERROR();

    // 2. 渲染 G-Buffer (GBufferPass)
    gBufferPass_->Render(rawMeshes, camera_);
    GL_CHECK_ERROR();

    oitPass_->Render(rawMeshes, camera_);
    GL_CHECK_ERROR();

    // // 3. 渲染 LightPass (直接光照)
    lightPass_->Render(gBufferPass_->getPositionTextureId(), // gPosition
                       gBufferPass_->getNormalTextureId(), // gNormal
                       gBufferPass_->getAlbedoTextureId(), // gAlbedo
                       gBufferPass_->getRoughnessTextureId(), // gRoughness
                       gBufferPass_->getMetallicTextureId(), // gMetallic
                       gBufferPass_->getAOTextureId(), // gAO
                       *mainLight_,
                       camera_,
                       shadowPass_->getShadowMapDepthOutputTextureID());
    GL_CHECK_ERROR();

    // // 4. 渲染 IBLPass (环境光照贡献)
    iblPass_->Render(gBufferPass_->getPositionTextureId(), // gPosition
                     gBufferPass_->getNormalTextureId(), // gNormal
                     gBufferPass_->getAlbedoTextureId(), // gAlbedo
                     gBufferPass_->getRoughnessTextureId(), // gRoughness
                     gBufferPass_->getMetallicTextureId(), // gMetallic
                     gBufferPass_->getAOTextureId(), // gAO
                     camera_);
    GL_CHECK_ERROR();

    // 5. 渲染天空盒 (SkyPass)
    // 注意：天空盒通常最后渲染，且不写入深度，因为它在所有物体后
    // 但为了确保深度测试正确进行，并且只渲染在可见区域，通常放在IBL后，Screen前。
    // 如果天空盒不写入深度，它将在前景物体后面。
    // skyPass_->Render(camera_);
    // GL_CHECK_ERROR();

    // 6. 最终的屏幕合成 Pass (ScreenPass)
    screenPass_->Render(lightPass_->getOutputTextureID(), // 直接光照结果
                        iblPass_->getOutputTexture(),       // IBL 环境光照结果
                        gBufferPass_->getDepthAttachment()); // G-Buffer 深度 (可能用于调试或后处理)
    GL_CHECK_ERROR();
}

void Scene::resize(int width, int height)
{
    // 更新 SceneData 的尺寸
    sceneData_.screenWidth = width;
    sceneData_.screenHeight = height;

    // 逐个调用所有 Pass 的 resize 方法
    if (gBufferPass_) {
        gBufferPass_->Resize(width, height);
    }
    if (lightPass_) {
        lightPass_->Resize(width, height);
    }
    // if (skyPass_) {
    //     skyPass_->Resize(width, height);
    // }
    if (screenPass_) {
        screenPass_->Resize(width, height);
    }
    if (iblPass_) {
        iblPass_->Resize(width, height);
    }
    // ShadowPass 通常只依赖于其固定的阴影贴图尺寸，不受屏幕尺寸影响
    // 但如果你的阴影 Pass 需要根据屏幕比例调整，这里也要调用其 Resize
    // if (shadowPass_) {
    //     shadowPass_->Resize(sceneData_.shadowMapWidth, sceneData_.shadowMapHeight);
    // }

    // 更新主相机的投影矩阵，以适应新的屏幕宽高比
    camera_.setAspectRatio(static_cast<float>(width) / height);
    // camera_.updateProjectionMatrix();
}