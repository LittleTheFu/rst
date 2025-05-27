#ifndef _SKY_PASS_H_
#define _SKY_PASS_H_

#include "RenderPass.h"     // 包含基类 RenderPass 的定义
#include "SceneData.h"      // 包含 SceneData 类的定义（用于访问 skybox）
#include "shader.h"
#include "camera.h"         // 包含 camera 头文件
#include "TextureCubeMap.h" // 包含 TextureCubeMap 类
#include <memory>           // 包含智能指针相关的头文件


class SkyPass : public RenderPass
{
public:
    // 构造函数：注入 Camera 引用和天空盒纹理
    SkyPass(int width, int height, std::shared_ptr<TextureCubeMap> skyboxTexture);
    ~SkyPass() override = default; // 析构函数（使用默认行为）

    // 实现基类的纯虚函数，不带参数
    void Render();

    // 重写基类的 Resize 方法
    void Resize(int width, int height) override;

    // 获取颜色输出纹理 ID
    GLuint getColorTexture() const { return colorTexture_; }

private:
    Shader shader_;

    // 天空盒渲染通常使用立方体模型，可以简化为直接在NDC空间渲染全屏四边形，
    // 或者使用一个立方体 VAO/VBO
    GLuint skyboxVAO_ = 0;
    GLuint skyboxVBO_ = 0;

    void initSkyboxCube(); // 初始化天空盒立方体
    void renderSkyboxCube(); // 渲染天空盒立方体

    // SkyPass 的输出纹理 ID (通常是颜色缓冲)
    GLuint colorTexture_ = 0; // 存储颜色信息
    GLuint depthRBO_ = 0;     // 存储深度信息

    // 通过构造函数注入并作为成员持有的 Camera 和天空盒纹理
    const Camera& camera_; // 存储 Camera 的引用
    std::shared_ptr<TextureCubeMap> skyboxTexture_; // 存储天空盒立方体贴图的智能指针
};

#endif // _SKY_PASS_H_