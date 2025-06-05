#include "MaterialFactory.h"
#include <iostream>

std::shared_ptr<Material> MaterialFactory::CreateMaterialFromDirectory(const std::string &name,
                                                                       const std::string &directory,
                                                                       const std::string &extension,
                                                                       bool flipY)
{
    auto material = std::make_shared<Material>(name);

    auto loadTex = [&](const std::string &type) -> std::shared_ptr<Texture2D>
    {
        std::string path = directory + "/" + type + extension;
        auto tex = Texture2D::loadFromFile(path, false, type == "albedo" ? true : false);
        if (!tex)
            std::cerr << "WARNING::MATERIAL_FACTORY::Failed to load " << type << " texture at " << path << std::endl;
        return tex;
    };

    material->setAlbedoMap(loadTex("albedo"));
    material->setNormalMap(loadTex("normal"));
    material->setRoughnessMap(loadTex("roughness"));
    material->setMetallicMap(loadTex("metallic"));
    material->setAmbientOcclusionMap(loadTex("ao"));

    return material;
}
