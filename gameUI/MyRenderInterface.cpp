#include "MyRenderInterface.h"
#include <iostream>

// 获取 MyRenderInterface 的唯一实例
MyRenderInterface& MyRenderInterface::getInstance() {
    static MyRenderInterface instance;
    return instance;
}

void MyRenderInterface::initialize() {
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // 默认使用 GL_LESS 深度函数

    // 启用背面剔除
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // 默认剔除背面

    // 启用 sRGB 帧缓冲，如果您的帧缓冲支持
    // glEnable(GL_FRAMEBUFFER_SRGB);

    // 默认清除颜色
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    GL_CHECK_ERROR(); // 检查初始化过程中的 OpenGL 错误
    std::cout << "MyRenderInterface: Initialized." << std::endl;
}

void MyRenderInterface::setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    GL_CHECK_ERROR();
}

void MyRenderInterface::clearBuffers() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GL_CHECK_ERROR();
}

void MyRenderInterface::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
    GL_CHECK_ERROR();
}

void MyRenderInterface::enable(GLenum capability) {
    glEnable(capability);
    GL_CHECK_ERROR();
}

void MyRenderInterface::disable(GLenum capability) {
    glDisable(capability);
    GL_CHECK_ERROR();
}

void MyRenderInterface::enableDepthTest(GLenum func) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(func);
    GL_CHECK_ERROR();
}

void MyRenderInterface::disableDepthTest() {
    glDisable(GL_DEPTH_TEST);
    GL_CHECK_ERROR();
}

void MyRenderInterface::enableBlending(GLenum sfactor, GLenum dfactor) {
    glEnable(GL_BLEND);
    glBlendFunc(sfactor, dfactor);
    GL_CHECK_ERROR();
}

void MyRenderInterface::disableBlending() {
    glDisable(GL_BLEND);
    GL_CHECK_ERROR();
}

void MyRenderInterface::enableCulling(GLenum mode) {
    glEnable(GL_CULL_FACE);
    glCullFace(mode);
    GL_CHECK_ERROR();
}

void MyRenderInterface::disableCulling() {
    glDisable(GL_CULL_FACE);
    GL_CHECK_ERROR();
}

void MyRenderInterface::setPolygonMode(GLenum face, GLenum mode) {
    glPolygonMode(face, mode);
    GL_CHECK_ERROR();
}

void MyRenderInterface::drawIndexed(const VertexArray& vao, const std::shared_ptr<Shader>& shader,
                                    GLsizei count, GLenum type, GLenum mode) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to draw with an invalid or uninitialized shader.");
    }
    vao.bind();
    shader->use();
    glDrawElements(mode, count, type, nullptr);
    GL_CHECK_ERROR();
    vao.unbind(); // 绘制完成后解绑 VAO
}

void MyRenderInterface::drawArrays(const VertexArray& vao, const std::shared_ptr<Shader>& shader,
                                   GLsizei vertexCount, GLenum mode, GLint first) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to draw with an invalid or uninitialized shader.");
    }
    vao.bind();
    shader->use();
    glDrawArrays(mode, first, vertexCount);
    GL_CHECK_ERROR();
    vao.unbind(); // 绘制完成后解绑 VAO
}

void MyRenderInterface::drawIndexedInstanced(const VertexArray& vao, const std::shared_ptr<Shader>& shader,
                                             GLsizei count, GLenum type, GLsizei instanceCount, GLenum mode) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to draw with an invalid or uninitialized shader.");
    }
    vao.bind();
    shader->use();
    glDrawElementsInstanced(mode, count, type, nullptr, instanceCount);
    GL_CHECK_ERROR();
    vao.unbind(); // 绘制完成后解绑 VAO
}

void MyRenderInterface::useShader(const std::shared_ptr<Shader>& shader) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to use an invalid or uninitialized shader.");
    }
    shader->use();
}

void MyRenderInterface::bindTexture(const std::shared_ptr<Texture2D>& texture, GLenum textureUnit) {
    if (!texture || texture->id() == 0) {
        THROW_GL_EXCEPTION("Attempted to bind an invalid or uninitialized 2D texture.");
    }
    texture->activate(textureUnit);
}

void MyRenderInterface::bindTexture(const std::shared_ptr<TextureCubeMap>& texture, GLenum textureUnit) {
    if (!texture || texture->id() == 0) {
        THROW_GL_EXCEPTION("Attempted to bind an invalid or uninitialized CubeMap texture.");
    }
    texture->activate(textureUnit);
}

void MyRenderInterface::setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, bool value) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to set uniform on an invalid or uninitialized shader.");
    }
    shader->setBool(name, value);
}

void MyRenderInterface::setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, int value) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to set uniform on an invalid or uninitialized shader.");
    }
    shader->setInt(name, value);
}

void MyRenderInterface::setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, float value) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to set uniform on an invalid or uninitialized shader.");
    }
    shader->setFloat(name, value);
}

void MyRenderInterface::setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, const Eigen::Vector2f& value) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to set uniform on an invalid or uninitialized shader.");
    }
    shader->setVec2(name, value);
}

void MyRenderInterface::setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, const Eigen::Vector3f& value) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to set uniform on an invalid or uninitialized shader.");
    }
    shader->setVec3(name, value);
}

void MyRenderInterface::setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, const Eigen::Vector4f& value) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to set uniform on an invalid or uninitialized shader.");
    }
    shader->setVec4(name, value);
}

void MyRenderInterface::setShaderUniform(const std::shared_ptr<Shader>& shader, const std::string& name, const Eigen::Matrix4f& value) {
    if (!shader || !shader->isValid()) {
        THROW_GL_EXCEPTION("Attempted to set uniform on an invalid or uninitialized shader.");
    }
    shader->setMat4(name, value);
}