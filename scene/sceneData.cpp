#include "sceneData.h"
#include "textureCubeMap.h"
#include "texture2D.h"
#include "materialFactory.h"
#include "model.h"
#include "sceneObject.h"
#include <textureManager.h>
#include <materialManager.h>

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

    sceneData->irradianceMapTex_ = std::move(TextureManager::getInstance().loadTextureCubeMap("ibl/house/houseDiffuseHDR.dds"));
    if (sceneData->irradianceMapTex_ == nullptr)
    {
        std::cerr << "ERROR::SCENE::Failed to load irradiance map! Check path and DDS format." << std::endl;
    }

    sceneData->prefilterMapTex_ = std::move(TextureManager::getInstance().loadTextureCubeMap("ibl/house/houseSpecularHDR.dds"));
    if (sceneData->prefilterMapTex_ == nullptr)
    {
        std::cerr << "ERROR::SCENE::Failed to load prefilter map! Check path and DDS format." << std::endl;
    }

    sceneData->brdfLUTTex_ = std::move(TextureManager::getInstance().loadTexture2D("ibl/house/houseBrdf.dds", true));
    if (sceneData->brdfLUTTex_ == nullptr)
    {
        std::cerr << "ERROR::SCENE::Failed to load BRDF LUT! Check path and DDS format." << std::endl;
    }

    std::shared_ptr<Material> goldMaterial = MaterialManager::getInstance().loadMaterial("goldMaterial", "gold");
    std::shared_ptr<Material> plasticMaterial = MaterialManager::getInstance().loadMaterial("plasticMaterial", "plastic");
    std::shared_ptr<Material> rustedIronMaterial = MaterialManager::getInstance().loadMaterial("rustedIronMaterial", "rusted_iron");
    std::shared_ptr<Material> grassMaterial = MaterialManager::getInstance().loadMaterial("grassMaterial", "grass");
    std::shared_ptr<Material> wallMaterial = MaterialManager::getInstance().loadMaterial("wallMaterial", "wall");
    std::shared_ptr<Material> silverMaterial = MaterialManager::getInstance().loadMaterial("silverMaterial", "silver");

    std::shared_ptr<Texture2D> gunAlbedoTexture = TextureManager::getInstance().loadTexture2D("gun/Textures/Cerberus_A.tga", false, true);
    std::shared_ptr<Texture2D> gunNormalTexture = TextureManager::getInstance().loadTexture2D("gun/Textures/Cerberus_N.tga");
    std::shared_ptr<Texture2D> gunRoughnessTexture = TextureManager::getInstance().loadTexture2D("gun/Textures/Cerberus_R.tga");
    std::shared_ptr<Texture2D> gunMetallicTexture = TextureManager::getInstance().loadTexture2D("gun/Textures/Cerberus_M.tga");
    std::shared_ptr<Texture2D> gunAoTexture = TextureManager::getInstance().loadTexture2D("gun/Textures/raw/Cerberus_AO.tga");

    std::shared_ptr<Material> gunMaterial = std::make_shared<Material>("gunMaterial");
    gunMaterial->setAlbedoMap(gunAlbedoTexture);
    gunMaterial->setNormalMap(gunNormalTexture);
    gunMaterial->setRoughnessMap(gunRoughnessTexture);
    gunMaterial->setMetallicMap(gunMetallicTexture);
    gunMaterial->setAmbientOcclusionMap(gunAoTexture);

    // float scale_gun = 0.02f;
    // std::unique_ptr<Model> gunModel = std::make_unique<Model>("gun/Cerberus_LP.FBX");
    // gunModel->setScale(Eigen::Vector3f(scale_gun, scale_gun, scale_gun));
    // gunModel->setRotation(Eigen::Quaternionf(M_PI / 4, 0.0f, 1.0f, 0.0f));
    // gunModel->setPosition(Eigen::Vector3f(4.0f, 3.0f, 2.0f));
    // sceneData->opaqueObjects.push_back(std::move(gunModel));

    float girlScale = 0.01f;
    std::unique_ptr<Model> girlModel = std::make_unique<Model>("XBot.fbx");
    girlModel->setScale(Eigen::Vector3f(girlScale, girlScale, girlScale));
    girlModel->setPosition(Eigen::Vector3f(-2.0f, 1.0f, 6.0f));
    sceneData->opaqueObjects.push_back(std::move(girlModel));

    float anmScale = 0.01f;
    std::unique_ptr<Model> anmModel = std::make_unique<Model>("StandingClap.fbx");
    anmModel->setScale(Eigen::Vector3f(anmScale, anmScale, anmScale));
    anmModel->setPosition(Eigen::Vector3f(2.0f, 1.0f, 6.0f));
    sceneData->opaqueObjects.push_back(std::move(anmModel));


    float ironTeapotScale = 1.0f;
    std::unique_ptr<Model> ironTeapotModel = std::make_unique<Model>("teapot.obj");
    ironTeapotModel->setMaterial(rustedIronMaterial);
    ironTeapotModel->setScale(Eigen::Vector3f(ironTeapotScale, ironTeapotScale, ironTeapotScale));
    ironTeapotModel->setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    sceneData->opaqueObjects.push_back(std::move(ironTeapotModel));

    float boxScale = 30.0f;
    std::unique_ptr<Model> boxModel = std::make_unique<Model>("bx.obj");
    boxModel->setMaterial(wallMaterial);
    boxModel->setPosition(Eigen::Vector3f(0.0f, 0.0f, -38.0f));
    boxModel->setScale(Eigen::Vector3f(boxScale, boxScale, boxScale));
    sceneData->opaqueObjects.push_back(std::move(boxModel));

    float grassTeapotScale = 1.0f;
    std::unique_ptr<Model> grassTeapotModel = std::make_unique<Model>("teapot.obj");
    grassTeapotModel->setMaterial(grassMaterial);
    grassTeapotModel->setScale(Eigen::Vector3f(grassTeapotScale, grassTeapotScale, grassTeapotScale));
    grassTeapotModel->setPosition(Eigen::Vector3f(0.0f, 4.0f, 0.0f));
    sceneData->opaqueObjects.push_back(std::move(grassTeapotModel));
    // sceneData->transparentObjects.push_back(std::move(grassTeapotModel));

    std::unique_ptr<Model> planeModelTop = std::make_unique<Model>("plane.obj");
    planeModelTop->setMaterial(silverMaterial);
    planeModelTop->setScale(Eigen::Vector3f(20.0f, 20.0f, 20.0f));
    planeModelTop->setPosition(Eigen::Vector3f(0.0f, -0.5f, -10.0f));
    sceneData->opaqueObjects.push_back(std::move(planeModelTop));

    std::unique_ptr<Model> planeModelBottom = std::make_unique<Model>("plane.obj");
    planeModelBottom->setMaterial(goldMaterial);
    planeModelBottom->setScale(Eigen::Vector3f(-20.0f, 20.0f, 20.0f));
    planeModelBottom->setPosition(Eigen::Vector3f(0.0f, -0.8f, -10.0f));
    sceneData->opaqueObjects.push_back(std::move(planeModelBottom));

    std::unique_ptr<Model> cursorModel = std::make_unique<Model>("bx.obj");
    cursorModel->setMaterial(plasticMaterial);
    cursorModel->setScale(Eigen::Vector3f(0.2f, 0.2f, 0.2f));
    sceneData->cursor = std::move(cursorModel);

    return sceneData;
}