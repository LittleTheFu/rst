// openglStateSaver.h
#ifndef OPENGL_STATE_SAVER_H
#define OPENGL_STATE_SAVER_H

#include <glad/glad.h> // 确保包含 GLAD 头文件

class OpenGLStateSaver {
public:
    // 构造函数：保存当前所有 OpenGL 状态
    OpenGLStateSaver() : stateSaved_(false) {
        // Save(); // 构造时自动保存所有状态
    }

    // 析构函数：如果状态被保存过且未被手动恢复，则自动恢复
    ~OpenGLStateSaver() {
        // if (stateSaved_) {
        //     Restore(); // 自动恢复
        // }
    }

    // 显式保存当前所有 OpenGL 状态
    // 注意：多次调用 Save() 会覆盖此对象中之前保存的状态。
    // 这与 RAII 的典型栈语义（先进后出）不同，请谨慎使用。
    void Save() {
        // 查询并保存所有你关心的 OpenGL 状态
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
        // 根据需要添加其他状态变量的保存
        // 例如：
        // glGetIntegerv(GL_BLEND_SRC_RGB, &lastBlendSrcRgb_);
        // glGetIntegerv(GL_BLEND_DST_RGB, &lastBlendDstRgb_);
        // glGetIntegerv(GL_BLEND_SRC_ALPHA, &lastBlendSrcAlpha_);
        // glGetIntegerv(GL_BLEND_DST_ALPHA, &lastBlendDstAlpha_);

        stateSaved_ = true; // 标记为已保存
    }

    // 显式恢复之前保存的 OpenGL 状态
    void Restore() {
        if (!stateSaved_) {
            // 没有保存的状态可以恢复，或者已经恢复过了
            return;
        }

        // 恢复之前保存的所有状态
        glUseProgram(lastProgramId_);
        if (lastBlendEnabled_) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (lastScissorEnabled_) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        glScissor(lastScissorBox_[0], lastScissorBox_[1], lastScissorBox_[2], lastScissorBox_[3]);
        if (lastCullFaceEnabled_) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (lastDepthTestEnabled_) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        glActiveTexture(static_cast<GLenum>(lastActiveTexture_)); // 确保 GLenum 类型匹配
        glBindTexture(GL_TEXTURE_2D, lastTextureBinding2D_);
        glBindVertexArray(lastVertexArrayBinding_);
        glViewport(lastViewport_[0], lastViewport_[1], lastViewport_[2], lastViewport_[3]);
        // 恢复其他状态
        // 例如：
        // glBlendFuncSeparate(lastBlendSrcRgb_, lastBlendDstRgb_, lastBlendSrcAlpha_, lastBlendDstAlpha_);

        stateSaved_ = false; // 标记为已恢复
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
    // GLint lastBlendSrcRgb_, lastBlendDstRgb_, lastBlendSrcAlpha_, lastBlendDstAlpha_;


    bool stateSaved_; // 记录状态是否已被保存，用于防止重复恢复
};

#endif // OPENGL_STATE_SAVER_H
