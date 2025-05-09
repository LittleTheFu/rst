#include "Shader.h"

Shader::Shader() : ID(0)
{
}

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
{
    load(vertexPath, fragmentPath); // 使用 load 函数
}

void Shader::load(const std::string &vertexPath, const std::string &fragmentPath)
{
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
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // 2. Compile shaders
    GLuint vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // Shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkProgramErrors(ID);

    // Delete the shaders as they're linked into our program now
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    uniformLocationCache_.clear();
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value)
{
    glUniform1i(getUniformLocation(name), (int)value);
}

void Shader::setInt(const std::string &name, int value)
{
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string &name, float value)
{
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string &name, const Eigen::Vector2f &value)
{
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, const Eigen::Vector3f &value)
{
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, const Eigen::Vector4f &value)
{
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setMat4(const std::string &name, const Eigen::Matrix4f &value)
{
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value.data());
}

GLint Shader::getUniformLocation(const std::string &name)
{
    if (uniformLocationCache_.find(name) != uniformLocationCache_.end())
    {
        return uniformLocationCache_[name];
    }

    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cerr << "WARNING::SHADER::UNIFORM '" << name << "' doesn't exist!" << std::endl;
    }
    uniformLocationCache_[name] = location;
    return location;
}

void Shader::checkCompileErrors(GLuint shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::COMPILE_FAILURE::" << type << "\n"
                      << infoLog << std::endl;
        }
    }
}

void Shader::checkProgramErrors(GLuint program)
{
    int success;
    char infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILURE\n"
                  << infoLog << std::endl;
    }
}