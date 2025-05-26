#include "scene.h"
#include <iostream>
#include <glad/glad.h>
#include "material.h"
#include "texture.h" // 包含你的通用 Texture 类 (用于LDR纹理，如albedo, normal等)
// #include "cubeMap.h" // 如果你不再使用旧的Cubemap类，可以删除
#include "debug_utils.h" // 确保包含调试工具

void Scene::init()
{
    // 1. 初始化场景数据和相机
    sceneData_.screenWidth = 800;
    sceneData_.screenHeight = 600;

    sceneData_.shadowMapWidth = 1024;
    sceneData_.shadowMapHeight = 1024;

    camera_.Position = Eigen::Vector3f(0.0f, 2.0f, 18.0f);
    camera_.lookAt(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    camera_.updateCameraVectors();

    // 初始化光源
    mainLight_ = std::make_shared<PointLight>();
    mainLight_->position = Eigen::Vector3f(0.0f, 0.0f, -30.0f);
    mainLight_->color = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    mainLight_->intensity = 8.0f;
    sceneData_.light = mainLight_; // 将主光源设置到 sceneData_ 中，供 ShadowPass 和 LightPass 使用

    // 2. 初始化阴影相机 (用于点光源阴影，通常需要六个方向的视图-投影矩阵，这里简化为一个)
    shadow_camera_.setAspectRatio(1.0f); // 阴影贴图通常是正方形
    shadow_camera_.setFOV(90.0f);        // 90度FOV用于点光源的立方体阴影贴图

    // 3. 加载 IBL 纹理 (在创建 IBLPass 和 SkyPass 之前加载)
    irradianceMapTex_ = std::make_shared<TextureCubeMap>();
    if (!irradianceMapTex_->loadDDS("ibl/house/houseDiffuseHDR.dds")) {
        std::cerr << "ERROR::SCENE::Failed to load irradiance map! Check path and DDS format." << std::endl;
        // 考虑加载一个默认的黑色纹理或终止程序
    }

    prefilterMapTex_ = std::make_shared<TextureCubeMap>();
    if (!prefilterMapTex_->loadDDS("ibl/house/houseSpecularHDR.dds")) {
        std::cerr << "ERROR::SCENE::Failed to load prefilter map! Check path and DDS format." << std::endl;
        // 考虑加载一个默认的黑色纹理或终止程序
    }

    brdfLUTTex_ = std::make_shared<Texture2D>();
    if (!brdfLUTTex_->loadDDS("ibl/house/houseBrdf.dds")) {
        std::cerr << "ERROR::SCENE::Failed to load BRDF LUT! Check path and DDS format." << std::endl;
        // 错误处理
    }

    // 4. 初始化渲染 Pass (现在使用新的构造函数参数)
    gBufferPass_ = std::make_unique<GBufferPass>(sceneData_.screenWidth, sceneData_.screenHeight, camera_, meshes_);
    shadowPass_ = std::make_unique<ShadowPass>(sceneData_.shadowMapWidth, sceneData_.shadowMapHeight, meshes_, mainLight_);

    // 注意：IBLPass 和 LightPass 的构造函数参数需要从 GBufferPass 的输出中获取
    // 它们在 init() 阶段还无法获取，因为 GBufferPass 还没渲染
    // 所以我们需要在 run() 中动态创建 LightPass/IBLPass/ScreenPass，或者修改它们的设计，
    // 让它们通过 setter 来接收输入纹理，而不是构造函数。
    // 为了遵循“构造函数注入”的原则，但又要在 init() 完成所有 Pass 初始化，
    // 我们需要预先假定 GBufferPass 的输出 ID。这是一个设计权衡。
    // 更常见的做法是让这些 Pass 有一个 setup() 或 initialize() 方法来接收这些后期可用的纹理。
    // 如果坚持构造函数注入，这里只能传递一些预设值或 placeholder，并在 run() 中设置。
    // 但为了避免复杂性，我们暂时让 GBufferPass 的输出在 init() 阶段就可访问（虽然它们尚未有数据）

    // 假设 GBufferPass 的 getColorAttachment() 在构造后就能提供有效的 GLuint ID（即使纹理内容为空）
    // 或者，我们可以将这些 Pass 的创建延迟到 run() 第一次执行之前，
    // 或者这些 Pass 的构造函数只接收尺寸和 camera，纹理通过 setter 传入。

    // **修正：** 鉴于 Pass 的设计，这里通过 getter 获取纹理 ID 是合理的，因为 ID 是 FBO 附件创建时就有的。
    // 但是，IBLPass 的 set 方法需要 TextureCubeMap 对象，不是 GLuint。

    iblPass_ = std::make_unique<IBLPass>(
        sceneData_.screenWidth, sceneData_.screenHeight,
        gBufferPass_->getColorAttachment(0), // gPosition
        gBufferPass_->getColorAttachment(1), // gNormal
        gBufferPass_->getColorAttachment(2), // gAlbedo
        gBufferPass_->getColorAttachment(3), // gRoughness
        gBufferPass_->getColorAttachment(4), // gMetallic
        gBufferPass_->getColorAttachment(5), // gAO
        camera_); // 传入 Camera
    iblPass_->setIrradianceMap(irradianceMapTex_);
    iblPass_->setPrefilterMap(prefilterMapTex_);
    iblPass_->setBrdfLUT(brdfLUTTex_); // 现在传递 Texture2D 共享指针

    // LightPass 同样
    lightPass_ = std::make_unique<LightPass>(
        sceneData_.screenWidth, sceneData_.screenHeight,
        gBufferPass_->getColorAttachment(0), // gPosition
        gBufferPass_->getColorAttachment(1), // gNormal
        gBufferPass_->getColorAttachment(2), // gAlbedo
        gBufferPass_->getColorAttachment(3), // gRoughness
        gBufferPass_->getColorAttachment(4), // gMetallic
        gBufferPass_->getColorAttachment(5), // gAO
        mainLight_, // 传入 PointLight 智能指针
        camera_,
        shadowPass_->getShadowTexture(), // 传入阴影纹理 ID
        shadow_camera_.GetLightSpaceMatrices()); // 传入阴影相机矩阵 (需要改造 LightPass 以接收这个)

    // ScreenPass 同样
    screenPass_ = std::make_unique<ScreenPass>(
        sceneData_.screenWidth, sceneData_.screenHeight,
        lightPass_->getColorAttachment(0), // lightTextureID (直接光照结果)
        iblPass_->getOutputTexture(),      // iblTextureID (IBL 结果)
        gBufferPass_->getDepthAttachment()); // lightDepthTextureID (G-Buffer 深度)

    // SkyPass
    // 注意：SkyPass 的 Render() 接收 Camera 和 TextureCubeMap
    skyPass_ = std::make_unique<SkyPass>(
        sceneData_.screenWidth, sceneData_.screenHeight,
        camera_,
        irradianceMapTex_); // 或 prefilterMapTex_，取决于天空盒材质的真实性需求

    // 5. 初始化网格和材质 (保持不变，或根据你的 Mesh/Material 系统调整)
    std::unique_ptr<Mesh> mesh_teapot = std::make_unique<Mesh>("teapot.obj");
    std::unique_ptr<Mesh> mesh_cursor = std::make_unique<Mesh>("bx.obj");
    std::unique_ptr<Mesh> mesh_box = std::make_unique<Mesh>("bx.obj");

    // LDR 纹理 (使用你现有的 Texture 类，它基于 stb_image)
    std::shared_ptr<Texture> albedoTexture = std::make_shared<Texture>("gold/albedo.png");
    std::shared_ptr<Texture> normalTexture = std::make_shared<Texture>("gold/normal.png");
    std::shared_ptr<Texture> roughnessTexture = std::make_shared<Texture>("gold/roughness.png");
    std::shared_ptr<Texture> metallicTexture = std::make_shared<Texture>("gold/metallic.png");
    std::shared_ptr<Texture> aoTexture = std::make_shared<Texture>("gold/ao.png");

    std::shared_ptr<Material> material_teapot = std::make_shared<Material>("teapot_mtrl");
    material_teapot->setAlbedoMap(albedoTexture);
    material_teapot->setNormalMap(normalTexture);
    material_teapot->setRoughnessMap(roughnessTexture);
    material_teapot->setMetallicMap(metallicTexture);
    material_teapot->setAmbientOcclusionMap(aoTexture);

    float mesh_box_scale = 10.0f;
    mesh_box->setMaterial(material_teapot);
    mesh_box->setPosition(Eigen::Vector3f(0.0f, 0.0f, -12.0f));
    mesh_box->setScale(Eigen::Vector3f(mesh_box_scale, mesh_box_scale, mesh_box_scale));

    mesh_teapot->setMaterial(material_teapot);
    float teapot_scale = 1.0f;
    mesh_teapot->setScale(Eigen::Vector3f(teapot_scale, teapot_scale, teapot_scale));
    mesh_teapot->setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));

    mesh_cursor->setMaterial(material_teapot);
    float cursor_scale = 0.2f;
    mesh_cursor->setScale(Eigen::Vector3f(cursor_scale, cursor_scale, cursor_scale));

    // 将网格添加到 meshes_ 成员中
    meshes_.push_back(std::move(mesh_box));
    meshes_.push_back(std::move(mesh_teapot));
    meshes_.push_back(std::move(mesh_cursor));

    // SkyPass 现在直接通过构造函数接收天空盒纹理，不再需要 SceneData 中的 skybox mesh
    // 但如果你想用一个 mesh 来渲染，你可以创建一个简单的立方体 mesh 给 SkyPass 的 VAO/VBO
    // 或像我之前改造 SkyPass 时，直接在 SkyPass 内部创建渲染立方体。
    // 如果你有一个 SkyBox mesh 并且它需要材质和纹理，你需要调整 SkyPass 构造函数
    // 让它接收 Mesh* 或 TextureCubeMap*。当前 SkyPass 直接绘制一个内部立方体。
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
    mainLight_->intensity = 10.0f;

    // 调试光标位置
    Eigen::Vector3f offset = Eigen::Vector3f(0.0f, 0.5f, 0.0f);
    if (meshes_.size() > 2)
    {
        meshes_.at(2)->setPosition(mainLight_->position + offset);
    }

    // 设置阴影相机 (更新位置)
    // 注意：ShadowPass 内部应该根据 PointLightData 计算六个方向的 light space matrices
    // 或者，如果你只实现一个方向的阴影，这里需要调整。
    // 假设 ShadowPass 内部会处理点光源的六个面。
    // 对于调试，我们可以简单地让 shadow_camera_ 追踪光源位置：
    shadow_camera_.Position = mainLight_->position;
    // shadow_camera_ 的 GetLightSpaceMatrices() 方法需要返回一个包含六个矩阵的 vector

    // --- 渲染管线执行 ---

    // 1. 渲染阴影贴图 (Shadow Pass)
    shadowPass_->Render();
    GL_CHECK_ERROR();

    // 2. 渲染 G-Buffer (GBufferPass)
    gBufferPass_->Render();
    GL_CHECK_ERROR();

    // 3. 渲染 LightPass (直接光照)
    // LightPass 现在通过构造函数接收所有纹理 ID 和光照数据
    // 并且它的 Render() 方法不再接收参数
    lightPass_->Render();
    GL_CHECK_ERROR();

    // 4. 渲染 IBLPass (环境光照贡献)
    iblPass_->Render();
    GL_CHECK_ERROR();

    // 5. 渲染天空盒 (SkyPass)
    // SkyPass 现在通过构造函数接收 Camera 和天空盒纹理，Render() 不带参数
    skyPass_->Render();
    GL_CHECK_ERROR();

    // 6. 最终的屏幕合成 Pass (ScreenPass)
    // ScreenPass 现在通过构造函数接收所有纹理 ID，Render() 不带参数
    screenPass_->Render();
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
    if (skyPass_) {
        skyPass_->Resize(width, height);
    }
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
    camera_.updateProjectionMatrix();
}