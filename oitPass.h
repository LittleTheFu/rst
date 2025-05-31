#ifndef _OIT_PASS_H
#define _OIT_PASS_H

#include "RenderPass.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h" // 确保包含 Mesh 头文件
#include <vector>
#include <memory> // 包含 shared_ptr 和 unique_ptr 的头文件
#include "texture2D.h"

class OitPass : public RenderPass
{
public:
    // 构造函数现在只负责 Pass 自身的初始化（如 FBO）
    OitPass(int width, int height);
    ~OitPass() override = default;

    // Render 方法现在明确接收其动态输入：网格列表和相机
    void Render(const std::vector<const Mesh*>& meshes, const Camera& camera, GLuint gPassDepthTextureID);

    void Resize(int width, int height) override;

    GLint getAccumTextureId() const;
    GLint getRevealTextureId() const;

private:
    Shader shader_;

    // G-Buffer 纹理的 IDs
    std::unique_ptr<Texture2D> accumTexture_; // 存储世界空间位置
    std::unique_ptr<Texture2D> revealTexture_;   // 存储世界空间法线
    
    std::unique_ptr<Texture2D> depthTexture_; // 存储深度信息（作为纹理）
    
    void init(); // 初始化 G-Buffer FBO 和纹理附件
};

#endif // GBUFFER_PASS_H