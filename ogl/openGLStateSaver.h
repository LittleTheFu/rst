// openglStateSaver.h
#ifndef OPENGL_STATE_SAVER_H
#define OPENGL_STATE_SAVER_H

#include <glad/glad.h> // 确保包含 GLAD 头文件

// 一个简单的 RAII 类，用于保存和恢复 OpenGL 状态
class OpenGLStateSaver {
public:
    // 构造函数：保存当前 OpenGL 状态
    OpenGLStateSaver() {
        // 查询并保存你关心的所有 OpenGL 状态
        glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgramId_);
        glGetBooleanv(GL_BLEND, &lastBlendEnabled_);
        glGetBooleanv(GL_SCISSOR_TEST, &lastScissorEnabled_);
        glGetIntegerv(GL_SCISSOR_BOX, lastScissorBox_);
        glGetBooleanv(GL_CULL_FACE, &lastCullFaceEnabled_);
        glGetBooleanv(GL_DEPTH_TEST, &lastDepthTestEnabled_);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &lastActiveTexture_);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureBinding2D_);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArrayBinding_);
        glGetIntegerv(GL_VIEWPORT, lastViewport_);
        // 如果你的渲染器还使用了其他状态，也在这里保存
    }

    // 析构函数：恢复之前保存的 OpenGL 状态
    ~OpenGLStateSaver() {
        // 恢复之前保存的所有状态
        glUseProgram(lastProgramId_);
        if (lastBlendEnabled_) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (lastScissorEnabled_) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        glScissor(lastScissorBox_[0], lastScissorBox_[1], lastScissorBox_[2], lastScissorBox_[3]);
        if (lastCullFaceEnabled_) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (lastDepthTestEnabled_) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        glActiveTexture(lastActiveTexture_);
        glBindTexture(GL_TEXTURE_2D, lastTextureBinding2D_);
        glBindVertexArray(lastVertexArrayBinding_);
        glViewport(lastViewport_[0], lastViewport_[1], lastViewport_[2], lastViewport_[3]);
        // 恢复其他状态
    }

private:
    // 存储需要保存的 OpenGL 状态变量
    GLint lastProgramId_;
    GLboolean lastBlendEnabled_;
    GLboolean lastScissorEnabled_;
    GLint lastScissorBox_[4];
    GLboolean lastCullFaceEnabled_;
    GLboolean lastDepthTestEnabled_;
    GLint lastActiveTexture_;
    GLint lastTextureBinding2D_;
    GLint lastVertexArrayBinding_;
    GLint lastViewport_[4];
    // 根据需要添加其他状态变量
};

#endif // OPENGL_STATE_SAVER_H
