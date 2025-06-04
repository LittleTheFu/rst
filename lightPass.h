#ifndef LIGHT_PASS_H
#define LIGHT_PASS_H

#include "RenderPass.h"    // 基类
#include "Shader.h"        // 着色器
#include "pointLight.h"    // 点光源
#include "uniformBuffer.h" // UBO
#include "camera.h"        // 相机
#include "Texture2D.h"     // G-Buffer 纹理输入和 LightPass 输出
#include "TextureCubeMap.h" // 阴影贴图可能需要 TextureCubeMap
#include "screenQuad.h"

class LightPass : public RenderPass
{
public:
    // 构造函数：注入所有 LightPass 所需的输入数据
    LightPass(int width, int height); // 阴影相机用于获取 lightSpaceMatrix 相关信息

    ~LightPass() override; // 析构函数，可能需要清理资源

    void Render(GLuint positionTextureID,
                GLuint normalTextureID,
                GLuint albedoTextureID,
                GLuint roughnessTextureID,
                GLuint metallicTextureID,
                GLuint aoTextureID,
                const PointLight &light,
                const Camera &camera,
                GLuint shadowMapID);

    // 重写 Resize 方法，处理内部纹理和 Framebuffer 的重新分配
    void Resize(int width, int height) override;

    GLuint getOutputTextureId() const { return outputTexture_->id(); }

    // (可选) 如果你仍然需要调试纹理的 getter，可以提供
    // const Texture2D& getDebugCurrentDepthTexture() const { return *debugCurrentDepthTexture_; }
    // const Texture2D& getDebugClosestDepthTexture() const { return *debugClosestDepthTexture_; }

private:
    Shader shader_;
    ScreenQuad screenQuad_;

    UniformBuffer objectLightUBO_;
    GLuint lightBindingPoint_;

    // LightPass 的输出纹理
    std::unique_ptr<Texture2D> outputTexture_;

    // 调试纹理（现在封装为 Texture2D 对象）
    std::unique_ptr<Texture2D> debugCurrentDepthTexture_;
    std::unique_ptr<Texture2D> debugClosestDepthTexture_;
};

#endif // LIGHT_PASS_H