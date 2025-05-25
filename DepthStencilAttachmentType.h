#ifndef DEPTH_STENCIL_ATTACHMENT_TYPE_H
#define DEPTH_STENCIL_ATTACHMENT_TYPE_H

// 定义枚举类型来表示深度/模板附件的配置选项
enum class DepthStencilAttachmentType {
    None,             // 不创建深度/模板附件
    Texture,          // 创建一个深度/模板纹理 (例如 GL_DEPTH24_STENCIL8)
    Renderbuffer      // 创建一个深度/模板渲染缓冲区 (例如 GL_DEPTH24_STENCIL8)
    // 未来可以扩展：
    // DepthTexture,    // 仅深度纹理 (如 GL_DEPTH_COMPONENT32F)
    // StencilTexture,  // 仅模板纹理 (如 GL_STENCIL_INDEX8)
    // DepthRenderbuffer, // 仅深度渲染缓冲区
    // StencilRenderbuffer // 仅模板渲染缓冲区
};

#endif // DEPTH_STENCIL_ATTACHMENT_TYPE_H