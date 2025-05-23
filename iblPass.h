#ifndef IBL_PASS_H
#define IBL_PASS_H

#include "RenderPass.h"
#include "Shader.h"
#include "camera.h" // 假设 camera.h 包含了 Camera 类定义

// 需要在你的项目中添加这些头文件，或者根据你的项目结构调整路径
// #include "textureLoader.h" // 假设你有一个加载DDS纹理的工具类

class IBLPass : public RenderPass
{
public:
    IBLPass(const std::string &name = "IBLPass");
    ~IBLPass() override = default;

    void Initialize(int width, int height) override;
    void Render(SceneData &sceneData, Camera &camera) override; // 不会使用sceneData,但为了override留着
    void Render(const GLuint &positionTextureID,
                const GLuint &normalTextureID,
                const GLuint &albedoTextureID,
                const GLuint &roughnessTextureID,
                const GLuint &metallicTextureID,
                const GLuint &aoTextureID,
                const Camera& camera); // 接收G-Buffer纹理和相机

    void Resize(int width, int height) override;

    // 获取输出纹理，通常是包含了IBL结果的颜色纹理
    GLuint getOutputTexture() const { return outputTexture_; }

    // 设置IBL纹理的方法
    void setIrradianceMap(GLuint textureID) { irradianceMap_ = textureID; }
    void setPrefilterMap(GLuint textureID) { prefilterMap_ = textureID; }
    void setBrdfLUT(GLuint textureID) { brdfLUT_ = textureID; }

private:
    GLuint quadVAO_ = 0;
    GLuint quadVBO_ = 0;

    void initScreenQuad();
    void renderQuad();

    // 输出纹理：IBL 计算结果
    GLuint outputTexture_;

    // IBL 所需的预计算纹理ID
    GLuint irradianceMap_ = 0; // 辐照度图 (HDR立方体贴图)
    GLuint prefilterMap_ = 0;  // 预过滤环境贴图 (HDR立方体贴图，带mipmaps)
    GLuint brdfLUT_ = 0;       // BRDF积分贴图 (2D纹理)

    // 着色器中用于采样 prefilterMap 的最大 LOD 级别
    // 这个值应该和你生成 prefilterMap 时使用的最大 mipmap 级别相匹配
    // 如果你使用 LearnOpenGL 的例子，128x128的立方体贴图有7个mipmap级别 (0-7)，
    // 但教程只渲染了5个级别(0-4)，所以MAX_REFLECTION_LOD是4.0。
    const float MAX_REFLECTION_LOD = 4.0f; // 根据你的prefilterMap实际mip级别设置

};

#endif // IBL_PASS_H