#ifndef IBL_PASS_H
#define IBL_PASS_H

#include "RenderPass.h"
#include "Shader.h"
#include "camera.h"
#include "shader.h"

// 包含新的 TextureCubeMap 类
#include "TextureCubeMap.h" 
// 如果你为2D浮点纹理（如BRDF LUT）创建了新的Texture2D类，也需要包含

class IBLPass : public RenderPass
{
public:
    IBLPass(const std::string &name = "IBLPass");
    ~IBLPass() override = default;

    void Initialize(int width, int height) override;
    void Render(SceneData &sceneData, Camera &camera) override;
    void Render(const GLuint &positionTextureID,
                const GLuint &normalTextureID,
                const GLuint &albedoTextureID,
                const GLuint &roughnessTextureID,
                const GLuint &metallicTextureID,
                const GLuint &aoTextureID,
                const Camera& camera);

    void Resize(int width, int height) override;

    GLuint getOutputTexture() const { return outputTexture_; }

    // 修改 set 方法，接收 TextureCubeMap 的共享指针，以便管理生命周期
    void setIrradianceMap(std::shared_ptr<TextureCubeMap> texture) { irradianceMap_ = texture; }
    void setPrefilterMap(std::shared_ptr<TextureCubeMap> texture) { prefilterMap_ = texture; }
    
    // BRDF LUT 仍然是 2D 纹理，如果它不是由 TextureCubeMap 管理，保持 GLuint
    // 但如果你的 brdfLUT 也是通过 gli 加载的 DDS 2D 纹理，你可能需要一个 Texture2D 类
    // 假设你有一个通用的 2D Texture 类，或者直接传递 GLuint
    void setBrdfLUT(GLuint textureID) { brdfLUT_ = textureID; } 

private:
    Shader shader_;

    GLuint quadVAO_ = 0;
    GLuint quadVBO_ = 0;

    void initScreenQuad();
    void renderQuad();

    GLuint outputTexture_;

    // IBL 所需的预计算纹理对象
    // 现在使用 std::shared_ptr 来持有 TextureCubeMap 实例
    std::shared_ptr<TextureCubeMap> irradianceMap_; // 辐照度图 (HDR立方体贴图)
    std::shared_ptr<TextureCubeMap> prefilterMap_;  // 预过滤环境贴图 (HDR立方体贴图，带mipmaps)
    
    GLuint brdfLUT_ = 0;       // BRDF积分贴图 (2D纹理，假设仍为 GLuint)

    const float MAX_REFLECTION_LOD = 4.0f; // 根据你的prefilterMap实际mip级别设置
};

#endif // IBL_PASS_H