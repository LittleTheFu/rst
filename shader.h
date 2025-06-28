#ifndef _SHADER_H_
#define _SHADER_H_

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <Eigen/Dense>

#include "asset.h" // 引入 IAsset 接口
#include "assetType.h" // 引入 AssetType 枚举

class Shader : public IAsset // <-- 继承 IAsset
{
public:
    GLuint ID;

    bool isValid() const;

    // 构造函数：现在会设置 m_id
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    Shader();

    virtual ~Shader();
    // load 函数现在会设置 m_id
    void load(const std::string& vertexPath, const std::string& fragmentPath);

    void use();
    void setBool(const std::string &name, bool value);
    void setInt(const std::string &name, int value);
    void setFloat(const std::string &name, float value);
    void setVec2(const std::string &name, const Eigen::Vector2f &value);
    void setVec3(const std::string &name, const Eigen::Vector3f &value);
    void setVec4(const std::string &name, const Eigen::Vector4f &value);
    void setMat4(const std::string &name, const Eigen::Matrix4f &value);

    GLuint getUniformBlockIndex(const std::string& blockName) const;
    void setUniformBlockBinding(GLuint blockIndex, GLuint bindingPoint);

    // IAsset 接口实现
    virtual const std::string& getID() const override { return m_id; }
    virtual AssetType getType() const override { return AssetType::Shader; } // <-- 返回 SHADER 类型

private:
    std::string m_id; // <-- 新增：存储着色器的唯一ID (通常是其路径组合)
    std::unordered_map<std::string, GLint> uniformLocationCache_;
    GLint getUniformLocation(const std::string &name);
    void checkCompileErrors(GLuint shader, std::string type);
    void checkProgramErrors(GLuint program);
};

#endif // _SHADER_H_