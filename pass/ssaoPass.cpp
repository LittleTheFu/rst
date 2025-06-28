// #include "ssaoPass.h"
// #include <random>

// SSAOPass::SSAOPass(int width, int height)
//     : RenderPass("SSAOPass", width, height), screenQuad_()
// {
//     ShaderManager::getInstance().loadShader("shader/ssao.vert", "shader/ssao.frag");

//     // 创建输出 AO 纹理
//     activateFramebuffer();
//     aoTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_R8);
//     frameBuffer_->attachColorTexture(aoTexture_->id(), GL_COLOR_ATTACHMENT0);
//     frameBuffer_->setDrawBuffers({GL_COLOR_ATTACHMENT0});
//     frameBuffer_->checkCompleteness();
//     deactivateFramebuffer();

//     // 生成采样核和噪声纹理
//     generateKernel();
//     generateNoiseTexture();
// }

// SSAOPass::~SSAOPass()
// {
//     glDeleteTextures(1, &noiseTextureID_);
// }

// void SSAOPass::generateKernel()
// {
//     std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
//     std::default_random_engine generator;

//     ssaoKernel_.clear();
//     for (int i = 0; i < 64; ++i)
//     {
//         Eigen::Vector3f sample(
//             randomFloats(generator) * 2.0f - 1.0f,
//             randomFloats(generator) * 2.0f - 1.0f,
//             randomFloats(generator)
//         );
//         sample = sample.normalized();
//         sample *= randomFloats(generator);

//         float scale = randomFloats(generator);
//         scale = 0.1f + (1.0f - 0.1f) * (scale * scale); // Eigen版 mix
//         sample *= scale;

//         ssaoKernel_.push_back(sample);
//     }
// }

// void SSAOPass::generateNoiseTexture()
// {
//     std::uniform_real_distribution<float> rand(-1.0, 1.0);
//     std::default_random_engine gen;
//     std::vector<Eigen::Vector3f> noise;

//     for (int i = 0; i < 16; ++i)
//         noise.emplace_back(rand(gen), rand(gen), 0.0f);

//     glGenTextures(1, &noiseTextureID_);
//     glBindTexture(GL_TEXTURE_2D, noiseTextureID_);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, noise.data());
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// }

// void SSAOPass::Render(GLuint positionTex, GLuint normalTex, const Eigen::Matrix4f &projectionMatrix)
// {
//     activateFramebuffer();
//     setViewport(width_, height_);
//     clearBuffers(GL_COLOR_BUFFER_BIT);

//     disableState(GL_DEPTH_TEST);

//     shader_.use();
//     shader_.setMat4("projection", projectionMatrix);

//     for (int i = 0; i < 64; ++i)
//         shader_.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel_[i]);

//     glBindTextureUnit(0, positionTex);
//     shader_.setInt("gPosition", 0);
//     glBindTextureUnit(1, normalTex);
//     shader_.setInt("gNormal", 1);
//     glBindTextureUnit(2, noiseTextureID_);
//     shader_.setInt("texNoise", 2);

//     screenQuad_.render();
//     deactivateFramebuffer();
// }

// void SSAOPass::Resize(int width, int height)
// {
//     RenderPass::Resize(width, height);
//     deactivateFramebuffer();
//     frameBuffer_ = std::make_unique<Framebuffer>(width_, height_);
//     activateFramebuffer();

//     aoTexture_ = std::make_unique<Texture2D>(Utilities::generateUniqueTextureId(),width_, height_, GL_R8);
//     frameBuffer_->attachColorTexture(aoTexture_->id(), GL_COLOR_ATTACHMENT0);
//     frameBuffer_->setDrawBuffers({GL_COLOR_ATTACHMENT0});
//     frameBuffer_->checkCompleteness();
//     deactivateFramebuffer();
// }
