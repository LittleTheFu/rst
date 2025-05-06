#ifndef _SHADER_H_
#define _SHADER_H_

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <Eigen/Dense>
#include <string>

class Shader
{
public:
    GLuint ID;

    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    void use();
    void setBool(const std::string &name, bool value);
    void setInt(const std::string &name, int value);
    void setFloat(const std::string &name, float value);
    void setVec2(const std::string &name, const Eigen::Vector2f &value);
    void setVec3(const std::string &name, const Eigen::Vector3f &value);
    void setVec4(const std::string &name, const Eigen::Vector4f &value);
    void setMat4(const std::string &name, const Eigen::Matrix4f &value);

private:
    std::unordered_map<std::string, GLint> uniformLocationCache_;
    GLint getUniformLocation(const std::string &name);
    void checkCompileErrors(GLuint shader, std::string type);
    void checkProgramErrors(GLuint program);
};

#endif // _SHADER_H_