#include "scene.h"
#include <iostream>
#include <glad/glad.h>
#include "material.h"
#include "texture.h" // 包含你的通用 Texture 类 (用于LDR纹理，如albedo, normal等)
#include "pointLight.h"
// #include "cubeMap.h" // 如果你不再使用旧的Cubemap类，可以删除

// 确保包含 TextureCubeMap.h 和 Texture2D.h，因为我们在 Scene 中管理它们的实例
#include "TextureCubeMap.h"
#include "Texture2D.h"
// 确保包含 IBLPass.h
#include "IBLPass.h"

void Scene::init()
{
    // 1. 初始化场景数据
    sceneData_.screenWidth = 800;
    sceneData_.screenHeight = 600;

    sceneData_.shadowMapWidth = 1024; // 阴影贴图的宽度和高度
    sceneData_.shadowMapHeight = 1024; // 阴影贴图的宽度和高度;

    // 2. 初始化渲染 Pass
    gBufferPass_ = std::make_unique<GBufferPass>();
    gBufferPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    lightPass_ = std::make_unique<LightPass>();
    lightPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    shadowPass_ = std::make_unique<ShadowPass>();
    shadowPass_->Initialize(sceneData_.shadowMapWidth, sceneData_.shadowMapHeight);

    skyPass_ = std::make_unique<SkyPass>();
    skyPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    screenPass_ = std::make_unique<ScreenPass>();
    screenPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    // 新增：初始化 IBL Pass
    iblPass_ = std::make_unique<IBLPass>();
    iblPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    // 3. 初始化相机
    camera_.Position = Eigen::Vector3f(0.0f, 2.0f, 18.0f);
    camera_.lookAt(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    camera_.updateCameraVectors();

    // 3. 初始化shadow_map相机 (保持不变)
    // shadow_camera_.Position = Eigen::Vector3f(1.0f, 3.0f, 20.0f);
    // shadow_camera_.Front = Eigen::Vector3f(0.0f, 0.0f, -1.0f);
    // shadow_camera_.updateCameraVectors();

    // 4. 初始化网格和材质
    std::unique_ptr<Mesh> mesh_teapot = std::make_unique<Mesh>("teapot.obj");
    std::unique_ptr<Mesh> mesh_cursor = std::make_unique<Mesh>("bx.obj");
    std::unique_ptr<Mesh> mesh_box = std::make_unique<Mesh>("bx.obj");
    std::unique_ptr<Mesh> mesh_sky = std::make_unique<Mesh>("bx.obj"); // 用于天空盒

    // LDR 纹理 (使用你现有的 Texture 类，它基于 stb_image)
    std::shared_ptr<Texture> albedoTexture = std::make_shared<Texture>("lena.png");
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

    // 旧的 Cubemap 加载（如果你使用 IBL，这个可能不再需要，或者 skyPass 需要修改）
    // std::shared_ptr<Cubemap> cubemapPtr = std::make_shared<Cubemap>(std::vector<std::string>{
    //     "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"
    // });
    // std::shared_ptr<Material> material_cubemap = std::make_shared<Material>("cubemap_mtrl");
    // material_cubemap->setCubemap(cubemapPtr);
    // sceneData_.skybox = cubemapPtr; // 如果你用IBL，这里的skybox可能变成环境图

    // --- 新增：加载 IBL 纹理 ---
    irradianceMapTex_ = std::make_shared<TextureCubeMap>();
    if (!irradianceMapTex_->loadDDS("homeDiffuseHDR.dds")) {
        std::cerr << "ERROR::SCENE::Failed to load irradiance map! Check path and DDS format." << std::endl;
        // 考虑加载一个默认的黑色纹理或终止程序
    }

    prefilterMapTex_ = std::make_shared<TextureCubeMap>();
    if (!prefilterMapTex_->loadDDS("homeSpecularHDR.dds")) {
        std::cerr << "ERROR::SCENE::Failed to load prefilter map! Check path and DDS format." << std::endl;
        // 考虑加载一个默认的黑色纹理或终止程序
    }

    // BRDF LUT 加载：现在使用新的 Texture2D 类
    brdfLUTTex_ = std::make_shared<Texture2D>();
    if (!brdfLUTTex_->loadDDS("homeBrdf.dds")) {
        std::cerr << "ERROR::SCENE::Failed to load BRDF LUT! Check path and DDS format." << std::endl;
        // 错误处理
    }

    // 将加载的 IBL 纹理传递给 IBLPass
    iblPass_->setIrradianceMap(irradianceMapTex_);
    iblPass_->setPrefilterMap(prefilterMapTex_);
    iblPass_->setBrdfLUT(brdfLUTTex_->getID()); // 传递 BRDF LUT 的 GLuint ID

    // 5. 初始化光源
    sceneData_.light = std::make_shared<PointLight>();
    sceneData_.light->position = Eigen::Vector3f(0.0f, 0.0f, -30.0f);
    sceneData_.light->color = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    sceneData_.light->intensity = 8.0f;

    // 6. 设置网格位置和缩放，并添加到场景数据
    float mesh_box_scale = 10.0f;
    mesh_box->setMaterial(material_teapot);
    mesh_box->setPosition(Eigen::Vector3f(0.0f, 0.0f, -12.0f));
    mesh_box->setScale(Eigen::Vector3f(mesh_box_scale, mesh_box_scale, mesh_box_scale));

    mesh_teapot->setMaterial(material_teapot);
    float teapot_scale = 1.0f;
    mesh_teapot->setScale(Eigen::Vector3f(teapot_scale, teapot_scale, teapot_scale));
    mesh_teapot->setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));

    // 如果你使用 IBL，天空盒的材质可能需要调整，以使用环境图
    // std::shared_ptr<Material> material_sky_ibl = std::make_shared<Material>("sky_ibl_mtrl");
    // material_sky_ibl->setCubemap(irradianceMapTex_); // 或原始的环境贴图
    // mesh_sky->setMaterial(material_sky_ibl);
    float sky_box_scale = 1.0f; // 或者更大以确保覆盖整个场景
    mesh_sky->setScale(Eigen::Vector3f(sky_box_scale, sky_box_scale, sky_box_scale));

    mesh_cursor->setMaterial(material_teapot);
    float cursor_scale = 0.2f;
    mesh_cursor->setScale(Eigen::Vector3f(cursor_scale, cursor_scale, cursor_scale));

    sceneData_.objects.push_back(std::move(mesh_box));
    sceneData_.objects.push_back(std::move(mesh_teapot));
    sceneData_.objects.push_back(std::move(mesh_cursor));
    sceneData_.skybox = std::move(mesh_sky); // 传递天空盒网格
}

void Scene::run()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // 背面剔除

    // for debug,test light---------------------------------------------
    static int count = 0;
    count++;
    count %= 48000;
    float x_light = count / 1200.0f - 15.0f;
    x_light *= 0.5;
    sceneData_.light->position = Eigen::Vector3f(x_light, x_light, 7.0f);
    sceneData_.light->intensity = 10.0f;

    // debug cursor position
    Eigen::Vector3f offset = Eigen::Vector3f(0.0f, 0.5f, 0.0f);
    if (sceneData_.objects.size() > 2)
    {
        sceneData_.objects.at(2)->setPosition(sceneData_.light->position + offset);
    }

    // 设置阴影相机
    shadow_camera_.Position = sceneData_.light->position;
    shadow_camera_.setAspectRatio(1); // 阴影贴图通常是正方形
    shadow_camera_.setFOV(90); // 90度FOV用于点光源的立方体阴影贴图

    // 渲染阴影贴图
    shadowPass_->Render(sceneData_, shadow_camera_);

    // lightSpaceMatrix 现在在 shadow_camera_ 内部处理
    // const Eigen::Matrix4f lightSpaceMatrix = shadow_camera_.GetProjectionMatrix() * shadow_camera_.GetViewMatrix();

    // 渲染 G-Buffer
    gBufferPass_->Render(sceneData_, camera_);

    // 渲染 LightPass (直接光照)
    lightPass_->Render(gBufferPass_->getColorAttachment(0), // gPosition
                       gBufferPass_->getColorAttachment(1), // gNormal
                       gBufferPass_->getColorAttachment(2), // gAlbedo
                       gBufferPass_->getColorAttachment(3), // gRoughness
                       gBufferPass_->getColorAttachment(4), // gMetallic
                       gBufferPass_->getColorAttachment(5), // gAO
                       sceneData_.light,
                       camera_,
                       shadowPass_->getShadowTexture(),
                       shadow_camera_);

    // 渲染 IBLPass (环境光照贡献)
    iblPass_->Render(gBufferPass_->getColorAttachment(0), // gPosition
                     gBufferPass_->getColorAttachment(1), // gNormal
                     gBufferPass_->getColorAttachment(2), // gAlbedo
                     gBufferPass_->getColorAttachment(3), // gRoughness
                     gBufferPass_->getColorAttachment(4), // gMetallic
                     gBufferPass_->getColorAttachment(5), // gAO
                     camera_);

    // 渲染天空盒
    // 如果你打算用 IBL 的原始环境图来渲染天空盒，这里需要修改 SkyPass 的 Render 方法
    // 例如：skyPass_->Render(environmentMapTex_->getID(), camera_);
    // 目前使用 sceneData_ 和 camera_，这意味着 SkyPass 内部会使用 sceneData_.skybox 的材质
    skyPass_->Render(sceneData_, camera_);


    // 最终的后处理/合成 Pass
    // ScreenPass 需要接收直接光照 (来自 lightPass_) 和 IBL 环境光照 (来自 iblPass_)，
    // 并在其着色器中进行 PBR 最终合成。
    // 这意味着你需要修改 ScreenPass 的着色器，以接收两个颜色纹理，并进行混合。
    screenPass_->Render(lightPass_->getColorAttachment(0), // 纹理 0：直接光照结果
                         iblPass_->getOutputTexture(),      // 纹理 1：IBL 环境光照结果
                         gBufferPass_->getDepthAttachment()); // 纹理 2：深度图 (可能用于其他后处理或调试)
}