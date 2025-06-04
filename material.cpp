#include "Material.h"

Material::Material(const std::string& name) : name_(name) {}

void Material::setAlbedoMap(const std::shared_ptr<Texture>& texture) {
    albedoMap_ = texture;
}

void Material::setNormalMap(const std::shared_ptr<Texture>& texture) {
    normalMap_ = texture;
}

void Material::setRoughnessMap(const std::shared_ptr<Texture>& texture) {
    roughnessMap_ = texture; 
}

void Material::setMetallicMap(const std::shared_ptr<Texture>& texture) {
    metallicMap_ = texture; 
}

void Material::setAmbientOcclusionMap(const std::shared_ptr<Texture>& texture) {
    aoMap_ = texture; 
}

// ... 其他纹理 setter ...

void Material::setAlbedoColor(const Eigen::Vector3f& color) {
    albedoColor_ = color;
}

void Material::setRoughnessFactor(float factor) {
    roughnessFactor_ = factor;
}

void Material::setMetallicFactor(float factor) {
    metallicFactor_ = factor;
}

// void Material::setCubemap(const std::shared_ptr<Cubemap> &cubemap)
// {
//     cubemap_ = cubemap;
// }

void Material::bindTextures(Shader& shader) {
    unsigned int textureUnit = 0;
    if (albedoMap_) {
        albedoMap_->activate(textureUnit);
        shader.setInt("albedoMap", textureUnit++);
        shader.setBool("hasAlbedoMap", true);
    } else {
        shader.setBool("hasAlbedoMap", false);
    }

    if (normalMap_) {
        normalMap_->activate(textureUnit);
        shader.setInt("normalMap", textureUnit++);
        shader.setBool("hasNormalMap", true);
    } else {
        shader.setBool("hasNormalMap", false);
    }

    if (roughnessMap_) {
        roughnessMap_->activate(textureUnit);
        shader.setInt("roughnessMap", textureUnit++);
        shader.setBool("hasRoughnessMap", true);
    } else {
        shader.setBool("hasRoughnessMap", false);
    }

    if (metallicMap_) {
        metallicMap_->activate(textureUnit);
        shader.setInt("metallicMap", textureUnit++);
        shader.setBool("hasMetallicMap", true);
    } else {
        shader.setBool("hasMetallicMap", false);
    }

    if (aoMap_) {
        aoMap_->activate(textureUnit);
        shader.setInt("aoMap", textureUnit++);
        shader.setBool("hasAoMap", true);
    } else {
        shader.setBool("hasAoMap", false);
    }

    // if (cubemap_) {
    //     cubemap_->use(textureUnit);
    //     shader.setInt("cubemap", textureUnit++);
    //     shader.setBool("hasCubemap", true); 
    // } else {
    //     shader.setBool("hasCubemap", false); 
    // }

    // ... 绑定其他纹理 ...
}

void Material::setUniforms(Shader& shader) {
    // shader.setVec3("material.albedoColor", albedoColor_);
    // shader.setFloat("material.roughnessFactor", roughnessFactor_);
    // shader.setFloat("material.metallicFactor", metallicFactor_);
}