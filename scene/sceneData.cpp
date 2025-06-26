#include "sceneData.h"
#include "textureCubeMap.h"
#include "texture2D.h"
#include "materialFactory.h"

std::unique_ptr<SceneData> sceneFactory::createScene()
{
    std::unique_ptr<SceneData> sceneData = std::make_unique<SceneData>();

    sceneData->screenWidth = 800;
    sceneData->screenHeight = 600;

    sceneData->shadowMapWidth = 1024;
    sceneData->shadowMapHeight = 1024;

    sceneData->camera = std::make_unique<Camera>();
    sceneData->camera->Position = Eigen::Vector3f(0.0f, 2.0f, 14.0f);
    sceneData->camera->lookAt(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    sceneData->camera->updateCameraVectors();

    sceneData->light = std::make_unique<PointLight>();
    sceneData->light->position = Eigen::Vector3f(0.0f, 0.0f, -30.0f);
    sceneData->light->color = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    sceneData->light->intensity = 8.0f;


    // 5. 加载 IBL 纹理 (在创建 IBLPass 和 SkyPass 之前加载)
    sceneData->irradianceMapTex_ = std::move(TextureCubeMap::loadDDS("ibl/house/houseDiffuseHDR.dds"));
    if (sceneData->irradianceMapTex_== nullptr) {
        std::cerr << "ERROR::SCENE::Failed to load irradiance map! Check path and DDS format." << std::endl;
    }

    sceneData->prefilterMapTex_ = std::move(TextureCubeMap::loadDDS("ibl/house/houseSpecularHDR.dds"));
    if (sceneData->prefilterMapTex_ == nullptr) {
        std::cerr << "ERROR::SCENE::Failed to load prefilter map! Check path and DDS format." << std::endl;
    }

    sceneData->brdfLUTTex_ = std::move(Texture2D::loadDDS("ibl/house/houseBrdf.dds"));
    if (sceneData->brdfLUTTex_ == nullptr) {
        std::cerr << "ERROR::SCENE::Failed to load BRDF LUT! Check path and DDS format." << std::endl;
    }

     //---create materials-----
    std::shared_ptr<Material> goldMaterial = MaterialFactory::CreateMaterialFromDirectory("goldMaterial", "gold");
    std::shared_ptr<Material> plasticMaterial = MaterialFactory::CreateMaterialFromDirectory("plasticMaterial", "plastic");
    std::shared_ptr<Material> rustedIronMaterial = MaterialFactory::CreateMaterialFromDirectory("rustedIronMaterial", "rusted_iron");
    std::shared_ptr<Material> grassMaterial = MaterialFactory::CreateMaterialFromDirectory("grassMaterial", "grass");
    std::shared_ptr<Material> wallMaterial = MaterialFactory::CreateMaterialFromDirectory("wallMaterial", "wall");
    std::shared_ptr<Material> silverMaterial = MaterialFactory::CreateMaterialFromDirectory("silverMaterial", "silver");

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

    float scale = 0.02f; // 调整缩放比例
    std::unique_ptr<Mesh> mesh_gun = std::make_unique<Mesh>("gun", "gun/Cerberus_LP.FBX");
    mesh_gun->setMaterial(gunMaterial);
    mesh_gun->setRotation(Eigen::Quaternionf(M_PI/4, 0.0f, 1.0f, 0.0f));
    mesh_gun->setScale(Eigen::Vector3f(scale, scale, scale));
    mesh_gun->setPosition(Eigen::Vector3f(4.0f, 3.0f, 2.0f));
    sceneData->opaqueObjects.push_back(std::move(mesh_gun));

     // 创建网格并设置材质和变换
    std::unique_ptr<Mesh> mesh_teapot_1 = std::make_unique<Mesh>("teapot_1", "teapot.obj");
    mesh_teapot_1->setMaterial(rustedIronMaterial);
    mesh_teapot_1->setScale(Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    mesh_teapot_1->setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    sceneData->opaqueObjects.push_back(std::move(mesh_teapot_1));
    // sceneData_.transparentObjects.push_back(std::move(mesh_teapot_1));

    std::unique_ptr<Mesh> mesh_box = std::make_unique<Mesh>("wall_box", "bx.obj");
    mesh_box->setMaterial(wallMaterial);
    mesh_box->setPosition(Eigen::Vector3f(0.0f, 0.0f, -38.0f));
    mesh_box->setScale(Eigen::Vector3f(12.0f, 12.0f, 12.0f));
    sceneData->opaqueObjects.push_back(std::move(mesh_box));

    std::unique_ptr<Mesh> mesh_teapot_2 = std::make_unique<Mesh>("teapot_2", "teapot.obj");
    mesh_teapot_2->setMaterial(grassMaterial);
    mesh_teapot_2->setScale(Eigen::Vector3f(4, 4, 4));
    mesh_teapot_2->setPosition(Eigen::Vector3f(0.0f, 0.0f, -30.0f));
    // sceneData->opaqueObjects.push_back(std::move(mesh_teapot_2));
    sceneData->transparentObjects.push_back(std::move(mesh_teapot_2));

    std::unique_ptr<Mesh> mesh_plane_top = std::make_unique<Mesh>("plane_top", "plane.obj");
    mesh_plane_top->setMaterial(silverMaterial);
    mesh_plane_top->setScale(Eigen::Vector3f(7.0f, 7.0f, 16.0f));
    mesh_plane_top->setPosition(Eigen::Vector3f(0.0f, -0.5f, -10.0f));
    sceneData->opaqueObjects.push_back(std::move(mesh_plane_top));
    // sceneData_.transparentObjects.push_back(std::move(mesh_plane_top));

    //a trick to make the plane shadow
    std::unique_ptr<Mesh> mesh_plane_bottom = std::make_unique<Mesh>("plane_bottom", "plane.obj");
    mesh_plane_bottom->setMaterial(goldMaterial);
    mesh_plane_bottom->setScale(Eigen::Vector3f(-7.0f, 7.0f, 16.0f));
    mesh_plane_bottom->setPosition(Eigen::Vector3f(0.0f, -0.8f, -10.0f));
    sceneData->opaqueObjects.push_back(std::move(mesh_plane_bottom));

    std::unique_ptr<Mesh> mesh_cursor = std::make_unique<Mesh>("cursor", "bx.obj");
    mesh_cursor->setMaterial(plasticMaterial);
    mesh_cursor->setScale(Eigen::Vector3f(0.2f, 0.2f, 0.2f));
    sceneData->opaqueObjects.push_back(std::move(mesh_cursor));

    return sceneData;
}