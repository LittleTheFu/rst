#ifndef _SKY_PASS_H_
#define _SKY_PASS_H_

#include "RenderPass.h"     // 包含基类 RenderPass 的定义
#include "SceneData.h"      // 包含 SceneData 类的定义（用于访问 skybox）
#include "shader.h"
#include "camera.h"         // 包含 camera 头文件
#include "TextureCubeMap.h" // 包含 TextureCubeMap 类
#include <memory>           // 包含智能指针相关的头文件
#include "skyBoxCube.h"     // 包含 SkyboxCube 类的定义
#include "texture2D.h"


class SkyPass : public RenderPass
{
public:
    // 构造函数：注入 Camera 引用和天空盒纹理
    SkyPass(int width, int height, std::shared_ptr<TextureCubeMap> skyboxTexture);
    ~SkyPass() override = default; // 析构函数（使用默认行为）

    // 实现基类的纯虚函数，不带参数
    void Render(const Camera &camera);

    // 重写基类的 Resize 方法
    void Resize(int width, int height) override;


private:
    void initFrameBuffer();

private:
    Shader shader_;
    SkyboxCube skyboxCube_;

    std::shared_ptr<TextureCubeMap> skyboxTexture_; // 天空盒纹理

    std::unique_ptr<Texture2D> colorTexture_; // 存储世界空间位置
    std::unique_ptr<Texture2D> depthTexture_; // 存储深度信息（作为纹理）
};

#endif // _SKY_PASS_H_