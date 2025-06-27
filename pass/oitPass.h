#ifndef _OIT_PASS_H
#define _OIT_PASS_H

#include "RenderPass.h"
#include "shader.h"
#include "camera.h"
#include "sceneObject.h" // !!! 引入 ISceneObject 接口，而不是 mesh.h
#include <vector>
#include <memory> // 包含 shared_ptr 和 unique_ptr 的头文件
#include "texture2D.h"
#include "textureCubeMap.h"
#include "pointLight.h"
#include "uniformBuffer.h"

class OitPass : public RenderPass
{
public:
    // 构造函数：注入所需的纹理
    OitPass(int width,
            int height,
            std::shared_ptr<TextureCubeMap> irradianceMap,
            std::shared_ptr<TextureCubeMap> prefilterMap,
            std::shared_ptr<Texture2D> brdfLUT);
    ~OitPass() override = default;

    // !!! 关键改动 !!!
    // Render 方法现在明确接收其动态输入：ISceneObject 列表、光源、相机和 G-Buffer 深度纹理ID
    void Render(const std::vector<ISceneObject*> &objects, // 传入 ISceneObject* 列表
                const PointLight &light,
                const Camera &camera,
                GLuint gPassDepthTextureID);

    void Resize(int width, int height) override;

    GLint getAccumTextureId() const;
    GLint getRevealTextureId() const;

private:
    Shader shader_;

    // OIT Accumulation Buffer 和 Revealage Buffer
    std::unique_ptr<Texture2D> accumTexture_;  // 存储累积的颜色和不透明度
    std::unique_ptr<Texture2D> revealTexture_; // 存储透明度信息

    // 你似乎在 OIT Pass 中创建了 depthTexture_，但又从 G-Buffer 传入深度纹理。
    // 如果 OIT Pass 需要独立的深度纹理，保留它。
    // 如果它仅使用 G-Buffer 的深度，那么这个 depthTexture_ 成员可能是不必要的，或者用于拷贝 G-Buffer 深度。
    // 这里暂时保留，但请你确认其用途。
    std::unique_ptr<Texture2D> depthTexture_; 

    // IBL 所需的预计算纹理对象 (通过构造函数注入)
    std::shared_ptr<TextureCubeMap> irradianceMap_; // 辐照度图 (HDR立方体贴图)
    std::shared_ptr<TextureCubeMap> prefilterMap_;  // 预过滤环境贴图 (HDR立方体贴图，带mipmaps)
    std::shared_ptr<Texture2D> brdfLUT_;            // BRDF积分贴图 (2D纹理)

    UniformBuffer objectLightUBO_;
    GLuint lightBindingPoint_;
    
    void init(); // 初始化 FBO 和纹理附件
};

#endif // _OIT_PASS_H