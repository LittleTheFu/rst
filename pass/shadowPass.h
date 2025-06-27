#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H

#include "RenderPass.h"     // 基类
#include "SceneData.h"      // 场景数据 (包含要渲染的对象) - 注意：SceneData 可能需要更新以存储 ISceneObject
#include "shader.h"         // 着色器
#include "Camera.h"         // 光源相机 (点光源通常使用立方体相机)
#include "TextureCubeMap.h" // 阴影立方体贴图
#include "sceneObject.h"   // !!! 引入 ISceneObject 接口

class ShadowPass : public RenderPass {
public:
    // 构造函数：注入 SceneData 和代表光源视角的 Camera
    ShadowPass(int width, int height);
    ~ShadowPass() override = default; // 析构函数，可能需要清理资源

    // !!! 关键改动 !!!
    // Render 方法现在接收 const std::vector<ISceneObject*>& 对象列表
    void Render(const std::vector<ISceneObject*>& objects, const PointLight& light);


    // 重写 Resize 方法，处理内部纹理和 Framebuffer 的重新分配
    void Resize(int width, int height) override;

    // 提供类型安全的阴影贴图 Getter
    GLuint getShadowMapDepthOutputTextureId() const { return shadowMapDepthOutputTexture_->id(); }

private:
    Shader shader_;

    // 阴影立方体贴图的封装对象
    std::unique_ptr<TextureCubeMap> shadowMapDepthTestTexture_;
    std::unique_ptr<TextureCubeMap> shadowMapDepthOutputTexture_;
    
    // 辅助函数：初始化 ShadowPass 内部的 Framebuffer 和纹理
    void initializeFramebufferAndTextures();
};

#endif // SHADOW_PASS_H