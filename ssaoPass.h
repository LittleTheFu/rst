// #ifndef SSAO_PASS_H
// #define SSAO_PASS_H

// #include "RenderPass.h"
// #include "Shader.h"
// #include "screenQuad.h"
// #include "Texture2D.h"
// #include <vector>
// #include <Eigen/Dense>

// class SSAOPass : public RenderPass
// {
// public:
//     SSAOPass(int width, int height);
//     ~SSAOPass() override;

//     void Render(GLuint positionTex, GLuint normalTex, const Eigen::Matrix4f &projectionMatrix);
//     void Resize(int width, int height) override;

//     GLuint getOutputTextureId() const { return aoTexture_->id(); }

// private:
//     void generateKernel();
//     void generateNoiseTexture();

//     Shader shader_;
//     ScreenQuad screenQuad_;
//     std::unique_ptr<Texture2D> aoTexture_;
//     std::unique_ptr<Texture2D> noiseTexture_;

//     std::vector<Eigen::Vector3f> ssaoKernel_;
//     GLuint noiseTextureID_ = 0;
// };

// #endif // SSAO_PASS_H
