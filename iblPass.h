#ifndef IBL_PASS_H
#define IBL_PASS_H

#include "RenderPass.h"
#include "Shader.h"
#include "camera.h" // 确保包含 camera 头文件

// 包含新的 Texture2D 和 TextureCubeMap 类
#include "Texture2D.h"
#include "TextureCubeMap.h"

class IBLPass : public RenderPass
{
public:
    // 通过构造函数注入 G-Buffer 纹理 ID 和 Camera
    IBLPass(int width, int height,
            std::shared_ptr<TextureCubeMap> irradianceMap,
            std::shared_ptr<TextureCubeMap> prefilterMap,
            std::shared_ptr<Texture2D> brdfLUT); // 接收 Camera 的引用

    ~IBLPass() override = default;

    // 实现基类的纯虚函数，不带参数
    void Render(GLuint gPositionID, GLuint gNormalID, GLuint gAlbedoID,
                GLuint gRoughnessID, GLuint gMetallicID, GLuint gAOID,
                const Camera& camera);

    // Resize 方法保持不变
    void Resize(int width, int height) override;

    // 获取输出纹理 ID
    GLuint getOutputTexture() const { return outputTexture_; }

    // 修改 set 方法，接收 TextureCubeMap 的共享指针，以便管理生命周期
    void setIrradianceMap(std::shared_ptr<TextureCubeMap> texture) { irradianceMap_ = texture; }
    void setPrefilterMap(std::shared_ptr<TextureCubeMap> texture) { prefilterMap_ = texture; }
    void setBrdfLUT(std::shared_ptr<Texture2D> texture) { brdfLUT_ = texture; }

private:
    Shader shader_;

    GLuint quadVAO_ = 0;
    GLuint quadVBO_ = 0;

    void initScreenQuad();
    void renderQuad();

    GLuint outputTexture_; // IBLPass 自身的输出纹理 ID (由其 FBO 管理)

    // IBL 所需的预计算纹理对象
    std::shared_ptr<TextureCubeMap> irradianceMap_; // 辐照度图 (HDR立方体贴图)
    std::shared_ptr<TextureCubeMap> prefilterMap_;  // 预过滤环境贴图 (HDR立方体贴图，带mipmaps)
    std::shared_ptr<Texture2D> brdfLUT_;           // BRDF积分贴图 (2D纹理)

    // MAX_REFLECTION_LOD 将从 prefilterMap_ 的实际 mip 级别中获取
};

#endif // IBL_PASS_H