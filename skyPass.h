#ifndef _SKY_PASS_H_
#define _SKY_PASS_H_

#include "RenderPass.h" // 包含基类 RenderPass 的定义
#include "SceneData.h"  // 包含 SceneData 类的定义

class SkyPass : public RenderPass
{ // GBufferPass 类继承自 RenderPass
public:
    SkyPass();            // 构造函数
    ~SkyPass() = default; // 析构函数（使用默认行为）

    void Initialize(int width, int height) override;            // 重写基类的 Initialize 方法
    void Render(SceneData &sceneData, Camera &camera) override; // 重写基类的 Render 方法

    GLuint getColorTexture() const { return colorTexture_; }

private:
    // G-Buffer 纹理的 IDs
    GLuint colorTexture_; // 存储位置信息

    GLuint depthRBO_;

};

#endif