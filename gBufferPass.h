#ifndef GBUFFERPASS_H
#define GBUFFERPASS_H

#include "RenderPass.h" // 包含基类 RenderPass 的定义
#include "SceneData.h"  // 包含 SceneData 类的定义

class GBufferPass : public RenderPass { // GBufferPass 类继承自 RenderPass
public:
    GBufferPass(); // 构造函数
    ~GBufferPass() = default; // 析构函数（使用默认行为）

    void Initialize(int width, int height) override; // 重写基类的 Initialize 方法
    void Render(SceneData& sceneData, Camera& camera) override; // 重写基类的 Render 方法

private:
    // G-Buffer 纹理的 IDs
    GLuint positionTexture_;           // 存储位置信息
    GLuint normalTexture_;             // 存储法线信息
    GLuint albedoSpecularTexture_;     // 存储反照率和镜面反射强度
    GLuint roughnessMetallicAOTexture_; // 存储粗糙度、金属度和环境光遮蔽信息
};

#endif // GBUFFERPASS_H