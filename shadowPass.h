#ifndef _SHADOW_PASS_H
#define _SHADOW_PASS_H

#include "RenderPass.h"
#include "SceneData.h"

class ShadowPass : public RenderPass { 
public:
    ShadowPass(); // 构造函数
    ~ShadowPass() = default; // 析构函数（使用默认行为）

    void Initialize(int width, int height) override; // 重写基类的 Initialize 方法
    void Render(SceneData& sceneData, Camera& camera) override; // 重写基类的 Render 方法

    GLuint getShadowTexture() { return colorAttachment_; }

private:
    GLuint colorAttachment_;           // 存储位置信息
};

#endif // ShadowPass_H