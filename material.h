#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <memory>
#include <Eigen/Dense>
#include "Texture.h"
#include "shader.h"

class Material {
public:
    Material(const std::string& name = "default_material");
    ~Material() = default;

    std::string getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

    // 纹理属性
    void setAlbedoMap(const std::shared_ptr<Texture>& texture);
    std::shared_ptr<Texture> getAlbedoMap() const { return albedoMap_; }

    void setNormalMap(const std::shared_ptr<Texture>& texture);
    std::shared_ptr<Texture> getNormalMap() const { return normalMap_; }

    void setRoughnessMap(const std::shared_ptr<Texture>& texture);
    std::shared_ptr<Texture> getRoughnessMap() const { return roughnessMap_; }

    void setMetallicMap(const std::shared_ptr<Texture>& texture);
    std::shared_ptr<Texture> getMetallicMap() const { return metallicMap_; }

    void setAmbientOcclusionMap(const std::shared_ptr<Texture>& texture);
    std::shared_ptr<Texture> getAmbientOcclusionMap() const { return aoMap_; }

    // ... 其他纹理属性的 setter/getter ...

    // 材质参数
    void setAlbedoColor(const Eigen::Vector3f& color);
    Eigen::Vector3f getAlbedoColor() const { return albedoColor_; }

    void setRoughnessFactor(float factor);
    float getRoughnessFactor() const { return roughnessFactor_; }

    void setMetallicFactor(float factor);
    float getMetallicFactor() const { return metallicFactor_; }

    // ... 其他材质参数的 setter/getter ...

    // 绑定材质到 Shader
    void bindTextures(Shader& shader);
    void setUniforms(Shader& shader); // 可选，用于设置非纹理 uniform

private:
    std::string name_;

    // 纹理
    std::shared_ptr<Texture> albedoMap_;
    std::shared_ptr<Texture> normalMap_;
    std::shared_ptr<Texture> roughnessMap_;
    std::shared_ptr<Texture> metallicMap_;
    std::shared_ptr<Texture> aoMap_;
    // ... 其他纹理 ...

    // 材质参数
    Eigen::Vector3f albedoColor_ = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    float roughnessFactor_ = 1.0f;
    float metallicFactor_ = 0.0f;
    // ... 其他参数 ...
};

#endif // MATERIAL_H