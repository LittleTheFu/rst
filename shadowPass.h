#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H

#include "RenderPass.h"     // 基类
#include "SceneData.h"      // 场景数据 (包含要渲染的对象)
#include "shader.h"         // 着色器
#include "Camera.h"         // 光源相机 (点光源通常使用立方体相机)
#include "TextureCubeMap.h" // 阴影立方体贴图

class ShadowPass : public RenderPass {
public:
    // 构造函数：注入 SceneData 和代表光源视角的 Camera
    // 注意：这里的 camera 参数是用于生成阴影贴图的“光源相机”，
    // 它的位置和视锥体将决定阴影的范围和质量。
    ShadowPass(int width, int height, SceneData& sceneData, Camera& lightCamera);
    ~ShadowPass() override; // 析构函数，可能需要清理资源

    // Render 方法现在明确接收其动态输入：要渲染的网格、光源和光照空间矩阵
    void Render(const std::vector<const Mesh*>& meshes, const PointLight& light, const std::vector<glm::mat4>& lightSpaceMatrices);


    // 重写 Resize 方法，处理内部纹理和 Framebuffer 的重新分配
    void Resize(int width, int height) override;

    // 提供类型安全的阴影贴图 Getter
    const TextureCubeMap& getShadowMapTexture() const { return *shadowMapTexture_; }

private:
    Shader shader_;

    // 阴影立方体贴图的封装对象
    std::unique_ptr<TextureCubeMap> shadowMapTexture_;

    // 对 SceneData 和光源相机的引用，通过构造函数注入
    SceneData& sceneData_;
    Camera& lightCamera_; // 代表光源视角的相机

    // 辅助函数：初始化 ShadowPass 内部的 Framebuffer 和纹理
    void initializeFramebufferAndTextures();
};

#endif // SHADOW_PASS_H