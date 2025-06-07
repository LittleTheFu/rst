#ifndef _OIT_PASS_H
#define _OIT_PASS_H

#include "RenderPass.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h" // 确保包含 Mesh 头文件
#include <vector>
#include <memory> // 包含 shared_ptr 和 unique_ptr 的头文件
#include "texture2D.h"
#include "textureCubeMap.h"
#include "pointLight.h"
#include "uniformBuffer.h"

class OitPass : public RenderPass
{
public:
    // 构造函数现在只负责 Pass 自身的初始化（如 FBO）
    OitPass(int width,
            int height,
            std::shared_ptr<TextureCubeMap> irradianceMap,
            std::shared_ptr<TextureCubeMap> prefilterMap,
            std::shared_ptr<Texture2D> brdfLUT);
    ~OitPass() override = default;

    // Render 方法现在明确接收其动态输入：网格列表和相机
    void Render(const std::vector<std::unique_ptr<Mesh>> &meshes,
                const PointLight &light,
                const Camera &camera,
                GLuint gPassDepthTextureID);

    void Resize(int width, int height) override;

    GLint getAccumTextureId() const;
    GLint getRevealTextureId() const;

private:
    Shader shader_;

    // G-Buffer 纹理的 IDs
    std::unique_ptr<Texture2D> accumTexture_; // 存储世界空间位置
    std::unique_ptr<Texture2D> revealTexture_;   // 存储世界空间法线
    
    std::unique_ptr<Texture2D> depthTexture_; // 存储深度信息（作为纹理）

     // IBL 所需的预计算纹理对象
    std::shared_ptr<TextureCubeMap> irradianceMap_; // 辐照度图 (HDR立方体贴图)
    std::shared_ptr<TextureCubeMap> prefilterMap_;  // 预过滤环境贴图 (HDR立方体贴图，带mipmaps)
    std::shared_ptr<Texture2D> brdfLUT_;           // BRDF积分贴图 (2D纹理)

    UniformBuffer objectLightUBO_;
    GLuint lightBindingPoint_;
    
    void init(); // 初始化 G-Buffer FBO 和纹理附件
};

#endif // GBUFFER_PASS_H