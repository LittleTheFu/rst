#include "shader.h"
#include <cassert>
#include "debug_utils.h" // 引入 GL_CHECK_ERROR()

Shader::Shader() : ID(0), m_id("") // 初始化 m_id
{
}

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
{
    load(vertexPath, fragmentPath); // 使用 load 函数
}

bool Shader::isValid() const
{
    return ID != 0;
}

void Shader::load(const std::string &vertexPath, const std::string &fragmentPath)
{
    // 如果已经有 ID，先删除旧程序以防重复加载
    if (ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
        uniformLocationCache_.clear();
    }

    // 设置着色器 ID，通常是其文件路径的组合
    m_id = vertexPath + "|" + fragmentPath;

    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // Ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        // Open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // Read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // Close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure &e)
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what()
                  << " -> Vertex: " << vertexPath << ", Fragment: " << fragmentPath << std::endl; // 增加路径信息
        // 加载失败，将 ID 设为 0，标记为无效
        ID = 0;
        return; // 加载失败，直接返回
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // 2. Compile shaders
    GLuint vertex, fragment;

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    GL_CHECK_ERROR(); // 检查编译错误

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    GL_CHECK_ERROR(); // 检查编译错误

    // Shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkProgramErrors(ID);
    GL_CHECK_ERROR(); // 检查链接错误

    // Delete the shaders as they're linked into our program now
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    GL_CHECK_ERROR(); // 检查删除错误

    uniformLocationCache_.clear();
}

// Shader 析构函数：释放 OpenGL 程序资源
Shader::~Shader() {
    if (ID != 0) {
        glDeleteProgram(ID);
        GL_CHECK_ERROR();
    }
}

void Shader::use()
{
    assert(ID != 0 && "Shader ID is 0, shader not initialized or loaded");
    glUseProgram(ID);
    GL_CHECK_ERROR(); // 检查 use 错误
}

void Shader::setBool(const std::string &name, bool value)
{
    glUniform1i(getUniformLocation(name), (int)value);
    GL_CHECK_ERROR();
}

void Shader::setInt(const std::string &name, int value)
{
    glUniform1i(getUniformLocation(name), value);
    GL_CHECK_ERROR();
}

void Shader::setFloat(const std::string &name, float value)
{
    glUniform1f(getUniformLocation(name), value);
    GL_CHECK_ERROR();
}

void Shader::setVec2(const std::string &name, const Eigen::Vector2f &value)
{
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
    GL_CHECK_ERROR();
}

void Shader::setVec3(const std::string &name, const Eigen::Vector3f &value)
{
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
    GL_CHECK_ERROR();
}

void Shader::setVec4(const std::string &name, const Eigen::Vector4f &value)
{
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
    GL_CHECK_ERROR();
}

void Shader::setMat4(const std::string &name, const Eigen::Matrix4f &value)
{
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value.data());
    GL_CHECK_ERROR();
}

GLuint Shader::getUniformBlockIndex(const std::string &blockName) const
{
    assert(ID != 0 && "Shader ID is 0, cannot get uniform block index."); // 确保 ID 有效
    GLuint index = glGetUniformBlockIndex(ID, blockName.c_str());
    // 使用 assert 检查是否找到了 Uniform Block
    assert(index != GL_INVALID_INDEX && ("Uniform Block '" + blockName + "' not found in shader program " + std::to_string(ID)).c_str());
    GL_CHECK_ERROR();
    return index;
}

// 设置 uniform block 的绑定点
void Shader::setUniformBlockBinding(GLuint blockIndex, GLuint bindingPoint)
{
    assert(ID != 0 && "Shader ID is 0, cannot set uniform block binding."); // 确保 ID 有效
    glUniformBlockBinding(ID, blockIndex, bindingPoint);
    GL_CHECK_ERROR();
}

GLint Shader::getUniformLocation(const std::string &name)
{
    if (uniformLocationCache_.find(name) != uniformLocationCache_.end())
    {
        return uniformLocationCache_[name];
    }

    assert(ID != 0 && "Shader ID is 0, cannot get uniform location."); // 确保 ID 有效
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cerr << "WARNING::SHADER::UNIFORM '" << name << "' doesn't exist in shader program " << ID << "!" << std::endl; // 增加程序ID
    }
    uniformLocationCache_[name] = location;
    GL_CHECK_ERROR(); // 检查 uniform location 错误
    return location;
}

void Shader::checkCompileErrors(GLuint shader, std::string type)
{
    int success;
    char infoLog[1024];
    // 检查着色器编译错误
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILE_FAILURE::" << type << "\n"
                  << infoLog << std::endl;
        // 编译失败时，可能需要将 ID 设为 0，以便 isValid() 返回 false
        if (type == "VERTEX" || type == "FRAGMENT") {
            // 这里我们只是打印错误，实际程序ID在link时才设置
            // 但如果一个shader编译失败，后续的链接肯定也会失败
        }
    }
}

void Shader::checkProgramErrors(GLuint program)
{
    int success;
    char infoLog[1024];
    // 检查程序链接错误
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILURE (Program ID: " << program << ")\n"
                  << infoLog << std::endl;
        // 链接失败时，将程序ID设为0，表示无效
        ID = 0; // 在这里设置 ID = 0，确保 isValid() 返回 false
    }
}

