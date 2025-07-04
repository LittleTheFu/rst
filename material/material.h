// Material.h (只显示修改的部分)
#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <memory>
#include <Eigen/Dense> // 确保 Eigen/Dense 路径正确
#include "Texture.h" // 确保 Texture.h 路径正确
#include "shader.h" // 确保 shader.h 路径正确
#include "asset.h" // 确保 asset.h 路径正确

class Material : public IAsset
{
public:
    Material(const std::string &name = "default_material");
    ~Material() = default;

public:
    // IAsset 接口实现
    // 将 name_ 用作 ID
    const std::string& getID() const override { return name_; } // <--- 修改这里
    AssetType getType() const override { return AssetType::Material; } // 返回材质类型
// private:
//     std::string assetId_; // <--- 移除这一行

public:
    std::string getName() const { return name_; }
    void setName(const std::string &name) { name_ = name; }

    void setAlbedoMap(const std::shared_ptr<Texture> &texture);
    std::shared_ptr<Texture> getAlbedoMap() const { return albedoMap_; }

    void setNormalMap(const std::shared_ptr<Texture> &texture);
    std::shared_ptr<Texture> getNormalMap() const { return normalMap_; }

    void setRoughnessMap(const std::shared_ptr<Texture> &texture);
    std::shared_ptr<Texture> getRoughnessMap() const { return roughnessMap_; }

    void setMetallicMap(const std::shared_ptr<Texture> &texture);
    std::shared_ptr<Texture> getMetallicMap() const { return metallicMap_; }

    void setAmbientOcclusionMap(const std::shared_ptr<Texture> &texture);
    std::shared_ptr<Texture> getAmbientOcclusionMap() const { return aoMap_; }

    void setAlbedoColor(const Eigen::Vector3f &color);
    Eigen::Vector3f getAlbedoColor() const { return albedoColor_; }

    void setRoughnessFactor(float factor);
    float getRoughnessFactor() const { return roughnessFactor_; }

    void setMetallicFactor(float factor);
    float getMetallicFactor() const { return metallicFactor_; }

    void bindTextures(Shader &shader);
    void setUniforms(Shader &shader);

private:
    std::string name_;

    std::shared_ptr<Texture> albedoMap_;
    std::shared_ptr<Texture> normalMap_;
    std::shared_ptr<Texture> roughnessMap_;
    std::shared_ptr<Texture> metallicMap_;
    std::shared_ptr<Texture> aoMap_;

    Eigen::Vector3f albedoColor_ = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    float roughnessFactor_ = 1.0f;
    float metallicFactor_ = 1.0f;
};

#endif