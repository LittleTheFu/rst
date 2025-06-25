#pragma once
#include <memory>
#include <string>
#include "material.h"
#include "texture2d.h"

class MaterialFactory
{
public:
    static std::shared_ptr<Material> CreateMaterialFromDirectory(const std::string &name,
                                                                 const std::string &directory,
                                                                 const std::string &extension = ".png",
                                                                 bool flipY = true);
};
