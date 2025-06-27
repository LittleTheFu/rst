#include "sceneData.h"
#include "textureCubeMap.h"
#include "texture2D.h"
#include "materialFactory.h"
#include <model.h>

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

    // quick and dirty, I will come bach here later...
    float scale = 0.02f; // 调整缩放比例
    std::unique_ptr<Model> gunModel = std::make_unique<Model>("gun/Cerberus_LP.FBX");
    gunModel->setScale(Eigen::Vector3f(scale, scale, scale));
    gunModel->setRotation(Eigen::Quaternionf(M_PI / 4, 0.0f, 1.0f, 0.0f));
    gunModel->setPosition(Eigen::Vector3f(4.0f, 3.0f, 2.0f));

    const std::vector<std::unique_ptr<Mesh>> &gunMeshes = gunModel->getMeshes();
    if (gunMeshes.size() > 0)
    {
        std::unique_ptr<Mesh> mesh = std::move(gunModel->takeMesh(0));
        // mesh->setMaterial(gunMaterial);
        sceneData->opaqueObjects.push_back(std::move(mesh));
    }

    // float scale = 0.02f; // 调整缩放比例
    // std::unique_ptr<Mesh> mesh_gun = std::make_unique<Mesh>("gun", "gun/Cerberus_LP.FBX");
    // mesh_gun->setMaterial(gunMaterial);
    // mesh_gun->setRotation(Eigen::Quaternionf(M_PI/4, 0.0f, 1.0f, 0.0f));
    // mesh_gun->setScale(Eigen::Vector3f(scale, scale, scale));
    // mesh_gun->setPosition(Eigen::Vector3f(4.0f, 3.0f, 2.0f));
    // sceneData->opaqueObjects.push_back(std::move(mesh_gun));

    std::unique_ptr<Model> ironTeapotModel = std::make_unique<Model>("teapot.obj");
    // quick and DANGEROUS!!!
    std::unique_ptr<Mesh> mesh_teapot_1 = ironTeapotModel->takeMesh(0);
    mesh_teapot_1->setMaterial(rustedIronMaterial);
    mesh_teapot_1->setScale(Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    mesh_teapot_1->setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    sceneData->opaqueObjects.push_back(std::move(mesh_teapot_1));

    // Load the box model once and then get its meshes
    std::unique_ptr<Model> boxModel = std::make_unique<Model>("bx.obj");
    if (!boxModel->getMeshes().empty())
    { // Ensure the model loaded successfully and has meshes
        // Create the first box (wall_box)
        std::unique_ptr<Mesh> mesh_box = boxModel->takeMesh(0); // Assuming 'bx.obj' has at least one mesh at index 0
        if (mesh_box)
        { // Check if takeMesh returned a valid mesh
            mesh_box->setMaterial(wallMaterial);
            mesh_box->setPosition(Eigen::Vector3f(0.0f, 0.0f, -38.0f));
            mesh_box->setScale(Eigen::Vector3f(12.0f, 12.0f, 12.0f));
            sceneData->opaqueObjects.push_back(std::move(mesh_box));
        }
    }
    else
    {
        std::cerr << "WARNING: bx.obj model has no meshes or failed to load." << std::endl;
    }

    // Load the teapot model once
    std::unique_ptr<Model> grassTeapotModel = std::make_unique<Model>("teapot.obj");
    if (!grassTeapotModel->getMeshes().empty())
    {
        // Create teapot_2
        std::unique_ptr<Mesh> mesh_teapot_2 = grassTeapotModel->takeMesh(0); // Assuming 'teapot.obj' has at least one mesh at index 0
        if (mesh_teapot_2)
        {
            mesh_teapot_2->setMaterial(grassMaterial);
            mesh_teapot_2->setScale(Eigen::Vector3f(4, 4, 4));
            mesh_teapot_2->setPosition(Eigen::Vector3f(0.0f, 0.0f, -30.0f));
            sceneData->transparentObjects.push_back(std::move(mesh_teapot_2));
        }
    }
    else
    {
        std::cerr << "WARNING: teapot.obj model has no meshes or failed to load." << std::endl;
    }

    // Load the plane model once
    std::unique_ptr<Model> planeModel = std::make_unique<Model>("plane.obj");
    if (!planeModel->getMeshes().empty())
    {
        // Create plane_top
        std::unique_ptr<Mesh> mesh_plane_top = planeModel->takeMesh(0); // Assuming 'plane.obj' has at least one mesh at index 0
        if (mesh_plane_top)
        {
            mesh_plane_top->setMaterial(silverMaterial);
            mesh_plane_top->setScale(Eigen::Vector3f(7.0f, 7.0f, 16.0f));
            mesh_plane_top->setPosition(Eigen::Vector3f(0.0f, -0.5f, -10.0f));
            sceneData->opaqueObjects.push_back(std::move(mesh_plane_top));
        }

        // Create plane_bottom (a trick for shadow)
        // Note: If you "take" a mesh, the Model no longer owns it. If you need multiple instances,
        // you might want a `cloneMesh` method, or load the 'plane.obj' model twice if it's very small.
        // For simplicity, sticking to `takeMesh` for now, assuming you only need one instance per Mesh type.
        // If plane.obj has multiple meshes, you could take another one, but if it only has one,
        // you'd need to reload the model or implement cloning.
        // For now, let's assume `plane.obj` has only one mesh, and you need a new instance, so we reload the model.
        // This is less efficient but adheres to `takeMesh(0)` for new instances.
        std::unique_ptr<Model> planeModelBottom = std::make_unique<Model>("plane.obj"); // Reload for another instance
        if (!planeModelBottom->getMeshes().empty())
        {
            std::unique_ptr<Mesh> mesh_plane_bottom = planeModelBottom->takeMesh(0);
            if (mesh_plane_bottom)
            {
                mesh_plane_bottom->setMaterial(goldMaterial);
                mesh_plane_bottom->setScale(Eigen::Vector3f(-7.0f, 7.0f, 16.0f)); // Note the negative scale
                mesh_plane_bottom->setPosition(Eigen::Vector3f(0.0f, -0.8f, -10.0f));
                sceneData->opaqueObjects.push_back(std::move(mesh_plane_bottom));
            }
        }
        else
        {
            std::cerr << "WARNING: plane.obj model (for bottom) has no meshes or failed to load." << std::endl;
        }
    }
    else
    {
        std::cerr << "WARNING: plane.obj model has no meshes or failed to load." << std::endl;
    }

    // Load the cursor model once
    std::unique_ptr<Model> cursorModel = std::make_unique<Model>("bx.obj"); // Assuming cursor is also a box
    if (!cursorModel->getMeshes().empty())
    {
        std::unique_ptr<Mesh> mesh_cursor = cursorModel->takeMesh(0);
        if (mesh_cursor)
        {
            mesh_cursor->setMaterial(plasticMaterial);
            mesh_cursor->setScale(Eigen::Vector3f(0.2f, 0.2f, 0.2f));
            sceneData->opaqueObjects.push_back(std::move(mesh_cursor));
        }
    }
    else
    {
        std::cerr << "WARNING: bx.obj model (for cursor) has no meshes or failed to load." << std::endl;
    }
    
    return sceneData;
}