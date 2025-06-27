#include "sceneData.h"
#include "textureCubeMap.h"
#include "texture2D.h"
#include "materialFactory.h"
#include "model.h" // 确保包含 Model.h，因为现在要创建 Model 对象
#include "sceneObject.h" // 如果你需要在列表中直接使用 ISceneObject*


std::unique_ptr<SceneData> sceneFactory::createScene()
{
    std::unique_ptr<SceneData> sceneData = std::make_unique<SceneData>();

    sceneData->screenWidth = 800;
    sceneData->screenHeight = 600;

    sceneData->shadowMapWidth = 1024;
    sceneData->shadowMapHeight = 1024;

    sceneData->camera = std::make_shared<Camera>();
    sceneData->camera->Position = Eigen::Vector3f(0.0f, 2.0f, 14.0f);
    sceneData->camera->lookAt(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    sceneData->camera->updateCameraVectors();

    sceneData->light = std::make_unique<PointLight>();
    sceneData->light->position = Eigen::Vector3f(0.0f, 0.0f, -30.0f);
    sceneData->light->color = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    sceneData->light->intensity = 8.0f;

    // 5. 加载 IBL 纹理 (在创建 IBLPass 和 SkyPass 之前加载)
    sceneData->irradianceMapTex_ = std::move(TextureCubeMap::loadDDS("ibl/house/houseDiffuseHDR.dds"));
    if (sceneData->irradianceMapTex_ == nullptr)
    {
        std::cerr << "ERROR::SCENE::Failed to load irradiance map! Check path and DDS format." << std::endl;
    }

    sceneData->prefilterMapTex_ = std::move(TextureCubeMap::loadDDS("ibl/house/houseSpecularHDR.dds"));
    if (sceneData->prefilterMapTex_ == nullptr)
    {
        std::cerr << "ERROR::SCENE::Failed to load prefilter map! Check path and DDS format." << std::endl;
    }

    sceneData->brdfLUTTex_ = std::move(Texture2D::loadDDS("ibl/house/houseBrdf.dds"));
    if (sceneData->brdfLUTTex_ == nullptr)
    {
        std::cerr << "ERROR::SCENE::Failed to load BRDF LUT! Check path and DDS format." << std::endl;
    }

    //---create materials-----
    std::shared_ptr<Material> goldMaterial = MaterialFactory::CreateMaterialFromDirectory("goldMaterial", "gold");
    std::shared_ptr<Material> plasticMaterial = MaterialFactory::CreateMaterialFromDirectory("plasticMaterial", "plastic");
    std::shared_ptr<Material> rustedIronMaterial = MaterialFactory::CreateMaterialFromDirectory("rustedIronMaterial", "rusted_iron");
    std::shared_ptr<Material> grassMaterial = MaterialFactory::CreateMaterialFromDirectory("grassMaterial", "grass");
    std::shared_ptr<Material> wallMaterial = MaterialFactory::CreateMaterialFromDirectory("wallMaterial", "wall");
    std::shared_ptr<Material> silverMaterial = MaterialFactory::CreateMaterialFromDirectory("silverMaterial", "silver");

    // 你也可以将这些纹理加载并设置给 Model 内部的材质，而不是在这里单独管理
    // 但如果你需要一个通用的 gunMaterial 供多个 gun 模型使用，这样是合理的
    std::shared_ptr<Texture2D> gunAlbedoTexture = Texture2D::loadFromFile("gun/Textures/Cerberus_A.tga", false, true);
    std::shared_ptr<Texture2D> gunNormalTexture = Texture2D::loadFromFile("gun/Textures/Cerberus_N.tga");
    std::shared_ptr<Texture2D> gunRoughnessTexture = Texture2D::loadFromFile("gun/Textures/Cerberus_R.tga");
    std::shared_ptr<Texture2D> gunMetallicTexture = Texture2D::loadFromFile("gun/Textures/Cerberus_M.tga");
    std::shared_ptr<Texture2D> gunAoTexture = Texture2D::loadFromFile("gun/Textures/raw/Cerberus_AO.tga");

    std::shared_ptr<Material> gunMaterial = std::make_shared<Material>("gunMaterial");
    gunMaterial->setAlbedoMap(gunAlbedoTexture);
    gunMaterial->setNormalMap(gunNormalTexture);
    gunMaterial->setRoughnessMap(gunRoughnessTexture);
    gunMaterial->setMetallicMap(gunMetallicTexture);
    gunMaterial->setAmbientOcclusionMap(gunAoTexture);


    // --- 关键改动：直接添加 Model 对象到 opaqueObjects ---
    float scale_gun = 0.02f; // 调整缩放比例
    std::unique_ptr<Model> gunModel = std::make_unique<Model>("gun/Cerberus_LP.FBX");
    gunModel->setScale(Eigen::Vector3f(scale_gun, scale_gun, scale_gun));
    gunModel->setRotation(Eigen::Quaternionf(M_PI / 4, 0.0f, 1.0f, 0.0f)); // Assimp Quaternions might be (w, x, y, z)
    gunModel->setPosition(Eigen::Vector3f(4.0f, 3.0f, 2.0f));
    // 注意：这里不再 takeMesh(0)，而是将整个 gunModel 添加进去
    // gunModel 已经加载了其内部所有的 mesh 和材质
    // 如果你希望 gunModel 使用上面自定义的 gunMaterial，你需要 Model 提供一个方法来替换其内部所有 mesh 的材质
    // gunModel->setMaterial(gunMaterial); // 如果 Model 有这个方法

    // 将整个 Model 加入到场景中
    sceneData->opaqueObjects.push_back(std::move(gunModel));


    std::unique_ptr<Model> ironTeapotModel = std::make_unique<Model>("teapot.obj");
    ironTeapotModel->setMaterial(rustedIronMaterial); // 将材质应用到 Model 内部所有 Mesh
    ironTeapotModel->setScale(Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    ironTeapotModel->setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    sceneData->opaqueObjects.push_back(std::move(ironTeapotModel)); // 加入整个 Model

    std::unique_ptr<Model> boxModel = std::make_unique<Model>("bx.obj");
    boxModel->setMaterial(wallMaterial);
    boxModel->setPosition(Eigen::Vector3f(0.0f, 0.0f, -38.0f));
    boxModel->setScale(Eigen::Vector3f(12.0f, 12.0f, 12.0f));
    sceneData->opaqueObjects.push_back(std::move(boxModel));

    std::unique_ptr<Model> grassTeapotModel = std::make_unique<Model>("teapot.obj");
    grassTeapotModel->setMaterial(grassMaterial);
    grassTeapotModel->setScale(Eigen::Vector3f(4, 4, 4));
    grassTeapotModel->setPosition(Eigen::Vector3f(0.0f, 0.0f, -30.0f));
    sceneData->transparentObjects.push_back(std::move(grassTeapotModel));

    std::unique_ptr<Model> planeModelTop = std::make_unique<Model>("plane.obj");
    planeModelTop->setMaterial(silverMaterial);
    planeModelTop->setScale(Eigen::Vector3f(7.0f, 7.0f, 16.0f));
    planeModelTop->setPosition(Eigen::Vector3f(0.0f, -0.5f, -10.0f));
    sceneData->opaqueObjects.push_back(std::move(planeModelTop));

    // 为底部平面重新加载模型，因为它是独立的对象，并且可能需要负缩放
    std::unique_ptr<Model> planeModelBottom = std::make_unique<Model>("plane.obj"); 
    planeModelBottom->setMaterial(goldMaterial);
    planeModelBottom->setScale(Eigen::Vector3f(-7.0f, 7.0f, 16.0f)); // Note the negative scale
    planeModelBottom->setPosition(Eigen::Vector3f(0.0f, -0.8f, -10.0f));
    sceneData->opaqueObjects.push_back(std::move(planeModelBottom));

    // Load the cursor model once
    std::unique_ptr<Model> cursorModel = std::make_unique<Model>("bx.obj"); // Assuming cursor is also a box
    cursorModel->setMaterial(plasticMaterial);
    cursorModel->setScale(Eigen::Vector3f(0.2f, 0.2f, 0.2f));
    // 直接将 Model 赋值给 sceneData->cursor，因为它现在也是 ISceneObject
    sceneData->cursor = std::move(cursorModel); 

    // Skybox 也是一个 Model
    // 假设你的 skybox 模型文件是 "skybox_model.obj" 或 "skybox_model.gltf"
    // std::unique_ptr<Model> skyboxModel = std::make_unique<Model>("skybox_model.obj");
    // skyboxModel->setMaterial(skyboxMaterial); // 如果 skybox 有特定材质
    // sceneData->skybox = std::move(skyboxModel);


    return sceneData;
}