#ifndef GBUFFER_PASS_H
#define GBUFFER_PASS_H

#include "RenderPass.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h" // 确保包含 Mesh 头文件
#include <vector>
#include <memory> // 包含 shared_ptr 和 unique_ptr 的头文件
#include "texture2D.h"

class GBufferPass : public RenderPass
{
public:
    // 构造函数现在只负责 Pass 自身的初始化（如 FBO）
    GBufferPass(int width, int height);
    ~GBufferPass() override = default;

    // Render 方法现在明确接收其动态输入：网格列表和相机
    void Render(const std::vector<std::shared_ptr<Mesh>>& meshes, const Camera& camera);
                
    void Resize(int width, int height) override;
    GLuint getDepthTextureId() const;

    // G-Buffer 纹理的 Getter 保持不变
    // GLuint getColorAttachment(int index) const;
    GLint getPositionTextureId() const;
    GLint getNormalTextureId() const;
    GLint getAlbedoTextureId() const;
    GLint getRoughnessTextureId() const;
    GLint getMetallicTextureId() const;
    GLint getAOTextureId() const;

private:
    Shader shader_;

    // G-Buffer 纹理的 IDs
    std::unique_ptr<Texture2D> positionTexture_; // 存储世界空间位置
    std::unique_ptr<Texture2D> normalTexture_;   // 存储世界空间法线
    std::unique_ptr<Texture2D> albedoTexture_;   // 存储反照率颜色和 AO
    std::unique_ptr<Texture2D> roughnessTexture_; // 存储粗糙度
    std::unique_ptr<Texture2D> metallicTexture_; // 存储金属度
    std::unique_ptr<Texture2D> aoTexture_;       // 存储环境光遮蔽 (AO)
    
    std::unique_ptr<Texture2D> depthTexture_; // 存储深度信息（作为纹理）
    
    void initGBuffer(); // 初始化 G-Buffer FBO 和纹理附件
};

#endif // GBUFFER_PASS_H