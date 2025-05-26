#ifndef GBUFFER_PASS_H
#define GBUFFER_PASS_H

#include "RenderPass.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h" // 确保包含 Mesh 头文件
#include <vector>

class GBufferPass : public RenderPass
{
public:
    // 构造函数现在只负责 Pass 自身的初始化（如 FBO）
    GBufferPass(int width, int height);
    ~GBufferPass() override = default;

    // Render 方法现在明确接收其动态输入：网格列表和相机
    void Render(const std::vector<const Mesh*>& meshes, const Camera& camera);

    void Resize(int width, int height) override;
    GLuint getDepthAttachment() const;

    // G-Buffer 纹理的 Getter 保持不变
    GLuint getColorAttachment(int index) const;

private:
    Shader shader_;

    // G-Buffer 纹理的 IDs
    GLuint gPosition_ = 0; // 存储世界空间位置
    GLuint gNormal_ = 0;   // 存储世界空间法线
    GLuint gAlbedo_ = 0;   // 存储反照率颜色和 AO
    GLuint gRoughness_ = 0; // 存储粗糙度
    GLuint gMetallic_ = 0; // 存储金属度
    GLuint gAO_ = 0;       // 存储环境光遮蔽 (AO)
    GLuint depthTexture_ = 0; // 存储深度信息（作为纹理）

    void initGBuffer(); // 初始化 G-Buffer FBO 和纹理附件

    // 不再在 GBufferPass 内部持有 meshes 和 camera 的引用
    // const Camera* camera_ = nullptr;
    // const std::vector<const Mesh*>* meshes_ = nullptr; // 使用指针防止所有权问题
};

#endif // GBUFFER_PASS_H