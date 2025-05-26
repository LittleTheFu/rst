#ifndef GBUFFERPASS_H
#define GBUFFERPASS_H

#include "RenderPass.h"    // 包含基类 RenderPass 的定义
#include "SceneData.h"     // 包含 SceneData 类的定义 (用于传入 SceneData& sceneData_)
#include "Camera.h"        // 包含 Camera 类的定义 (用于传入 Camera& camera_)
#include "shader.h"        // 包含 Shader 类
#include "Texture2D.h"     // 包含 Texture2D 类，用于 G-Buffer 附件

// 前向声明，以避免循环依赖，如果 Mesh, Material 等在其他头文件中
// class Mesh;
// class Material;
// ... (根据你的实际依赖添加)

class GBufferPass : public RenderPass {
public:
    // 构造函数：注入 SceneData 和 Camera 的引用。
    // GBufferPass 需要这些来知道渲染什么以及如何渲染 (模型、视图、投影矩阵)。
    GBufferPass(int width, int height, SceneData& sceneData, Camera& camera);
    ~GBufferPass() = default; // 析构函数（使用默认行为）

    // 重写基类的 Render 方法，现在不带参数。
    // GBufferPass 将使用其内部持有的 sceneData_ 和 camera_ 来渲染。
    void Render() override;

    // --- 提供类型安全的 G-Buffer 纹理 Getter ---
    // 这些 Getter 返回 const 引用，确保外部代码不能修改纹理对象本身，
    // 并且避免了不必要的拷贝。
    const Texture2D& getPositionTexture() const { return *positionTexture_; }
    const Texture2D& getNormalTexture() const { return *normalTexture_; }
    const Texture2D& getAlbedoTexture() const { return *albedoTexture_; }
    const Texture2D& getRoughnessTexture() const { return *roughnessTexture_; }
    const Texture2D& getMetallicTexture() const { return *metallicTexture_; }
    const Texture2D& getAOTexture() const { return *aoTexture_; }
    const Texture2D& getDepthTexture() const { return *depthTexture_; } // 深度纹理的 Getter

    // (可选) 如果你需要重新调整 G-Buffer 尺寸，这里需要重写 Resize 方法。
    // GBufferPass 内部的纹理也需要重新分配存储。
    void Resize(int width, int height) override;

private:
    Shader shader_;

    // G-Buffer 纹理的封装对象，使用 unique_ptr 管理它们的生命周期。
    std::unique_ptr<Texture2D> positionTexture_;
    std::unique_ptr<Texture2D> normalTexture_;
    std::unique_ptr<Texture2D> albedoTexture_;
    std::unique_ptr<Texture2D> roughnessTexture_;
    std::unique_ptr<Texture2D> metallicTexture_;
    std::unique_ptr<Texture2D> aoTexture_;
    std::unique_ptr<Texture2D> depthTexture_; // 深度纹理

    // 对 SceneData 和 Camera 的引用，通过构造函数注入。
    // 注意：这里使用引用，所以确保传入的 SceneData 和 Camera 对象
    // 在 GBufferPass 生命周期内是有效且不变的。
    SceneData& sceneData_;
    Camera& camera_;

    // 辅助函数：在构造和 Resize 时创建并附加单个颜色纹理到 Framebuffer。
    // 这是一个内部实现细节，不应在头文件中暴露给外部直接调用。
    void createAndAttachColorTexture(std::unique_ptr<Texture2D>& texturePtr, GLenum internalFormat, GLenum attachmentPoint);
};

#endif // GBUFFERPASS_H