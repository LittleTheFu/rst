#ifndef GOD_RAY_PASS_H
#define GOD_RAY_PASS_H

#include "RenderPass.h"
#include "shader.h"
#include "texture2D.h"
#include <memory>
#include "screenQuad.h"

class GodRayPass : public RenderPass
{
public:
    GodRayPass(int width, int height);
    ~GodRayPass() override = default;

    void Render(GLuint brightnessTexture,
                const Eigen::Vector3f &lightPos,
                const Eigen::Matrix4f &viewMatrix,
                const Eigen::Matrix4f &projMatrix,
                float exposure,
                float decay,
                float density,
                float weight,
                int samples);

    GLuint getColorTextureId() const;

    void Resize(int width, int height) override;

private:
    void initFramebuffer();

    std::shared_ptr<Shader> shader_;
    std::unique_ptr<Texture2D> colorTexture_; // 输出贴图

    ScreenQuad screenQuad_; // 用于渲染全屏四边形
};

#endif // GOD_RAY_PASS_H
