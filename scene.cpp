#include "scene.h"
#include <iostream>
#include <glad/glad.h>
#include "debug_utils.h"


void Scene::blur(bool isOn)
{
    isBlurOn_ = isOn;
}

void Scene::init()
{
    sceneData_ = std::move(sceneFactory::createScene());

    skyPass_ = std::make_unique<SkyPass>(sceneData_->screenWidth, sceneData_->screenHeight, sceneData_->prefilterMapTex_);
    gBufferPass_ = std::make_unique<GBufferPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    shadowPass_ = std::make_unique<ShadowPass>(sceneData_->shadowMapWidth, sceneData_->shadowMapHeight);
    lightPass_ = std::make_unique<LightPass>(sceneData_->screenWidth, sceneData_->screenHeight);

    oitPass_ = std::make_unique<OitPass>(
        sceneData_->screenWidth,
        sceneData_->screenHeight,
        sceneData_->irradianceMapTex_,
        sceneData_->prefilterMapTex_,
        sceneData_->brdfLUTTex_);

    iblPass_ = std::make_unique<IBLPass>(
        sceneData_->screenWidth,
        sceneData_->screenHeight,
        sceneData_->irradianceMapTex_,
        sceneData_->prefilterMapTex_,
        sceneData_->brdfLUTTex_);

    combinedPass_ = std::make_unique<CombinedPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    ssrPass_ = std::make_unique<SSRPass>(sceneData_->screenWidth, sceneData_->screenHeight);

    blurHorizontalPass_ = std::make_unique<BlurHorizontalPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    blurVerticalPass_ = std::make_unique<BlurVerticalPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    depthOfFieldPass_ = std::make_unique<DepthOfFieldPass>(sceneData_->screenWidth, sceneData_->screenHeight);

    postPass_ = std::make_unique<PostPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    screenPass_ = std::make_unique<ScreenPass>(sceneData_->screenWidth, sceneData_->screenHeight);
}

void Scene::run()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // 背面剔除

    // 光源调试动画
    static int count = 0;
    count++;
    count %= 8000;
    float x_light = count / 1200.0f - 3.0f;
    x_light *= 1.0f;
    sceneData_->light->position = Eigen::Vector3f(x_light, x_light, 3.0f);
    // mainLight_->position = Eigen::Vector3f(5, 5, 7.0f);
    sceneData_->light->intensity = 8.0f;

    // 调试光标位置
    Eigen::Vector3f offset = Eigen::Vector3f(0.0f, 0.5f, 0.0f);
    if(!sceneData_->opaqueObjects.empty())
    {
        sceneData_->opaqueObjects.back()->setPosition(sceneData_->light->position + offset);
    }

    // --- 渲染管线执行 ---

    skyPass_->Render(*sceneData_->camera);
    GL_CHECK_ERROR();


    shadowPass_->Render(sceneData_->opaqueObjects, *sceneData_->light);
    GL_CHECK_ERROR();

    gBufferPass_->Render(sceneData_->opaqueObjects, *sceneData_->camera);
    GL_CHECK_ERROR();

    oitPass_->Render(sceneData_->transparentObjects,
                     *sceneData_->light,
                     *sceneData_->camera,
                     gBufferPass_->getDepthTextureId());
    GL_CHECK_ERROR();

    lightPass_->Render(gBufferPass_->getPositionTextureId(), // gPosition
                       gBufferPass_->getNormalTextureId(), // gNormal
                       gBufferPass_->getAlbedoTextureId(), // gAlbedo
                       gBufferPass_->getRoughnessTextureId(), // gRoughness
                       gBufferPass_->getMetallicTextureId(), // gMetallic
                       gBufferPass_->getAOTextureId(), // gAO
                       *sceneData_->light,
                       *sceneData_->camera,
                       shadowPass_->getShadowMapDepthOutputTextureId());
    GL_CHECK_ERROR();

    iblPass_->Render(gBufferPass_->getPositionTextureId(), // gPosition
                     gBufferPass_->getNormalTextureId(), // gNormal
                     gBufferPass_->getAlbedoTextureId(), // gAlbedo
                     gBufferPass_->getRoughnessTextureId(), // gRoughness
                     gBufferPass_->getMetallicTextureId(), // gMetallic
                     gBufferPass_->getAOTextureId(), // gAO
                     *sceneData_->camera);
    GL_CHECK_ERROR();

    combinedPass_->Render(lightPass_->getOutputTextureId(),
                        iblPass_->getColorTextureId(),
                        gBufferPass_->getDepthTextureId(),
                        oitPass_->getAccumTextureId(),
                        oitPass_->getRevealTextureId(),
                        skyPass_->getColorTextureId(),
                        ssrPass_->getReflectionTextureId());
    GL_CHECK_ERROR();

    ssrPass_->Render(gBufferPass_->getNormalTextureId(),
                    gBufferPass_->getDepthTextureId(),
                    gBufferPass_->getAlbedoTextureId(),
                    gBufferPass_->getMetallicTextureId(),
                    gBufferPass_->getRoughnessTextureId(),
                    sceneData_->camera->GetProjectionMatrix(),
                    sceneData_->camera->GetViewMatrix());

    if (isBlurOn_)
    {
        blurHorizontalPass_->Render(combinedPass_->getColorTextureId());
        GL_CHECK_ERROR();

        blurVerticalPass_->Render(blurHorizontalPass_->getColorTextureId());
        GL_CHECK_ERROR();

        depthOfFieldPass_->Render(combinedPass_->getColorTextureId(),
                                  blurVerticalPass_->getColorTextureId(),
                                  gBufferPass_->getDepthTextureId(),
                                  18.0f,
                                  10.0f,
                                  sceneData_->camera->nearClip,
                                  sceneData_->camera->farClip);

        postPass_->Render(depthOfFieldPass_->getColorTextureId());
        GL_CHECK_ERROR();
    }
    else
    {
        postPass_->Render(combinedPass_->getColorTextureId());
        GL_CHECK_ERROR();
    }

    screenPass_->Render(postPass_->getColorTextureId());
    GL_CHECK_ERROR();
}

void Scene::resize(int width, int height)
{
    // 更新 SceneData 的尺寸
    sceneData_->screenWidth = width;
    sceneData_->screenHeight = height;

    // 逐个调用所有 Pass 的 resize 方法
    if (gBufferPass_)
    {
        gBufferPass_->Resize(width, height);
    }

    if (lightPass_)
    {
        lightPass_->Resize(width, height);
    }

    if (skyPass_)
    {
        skyPass_->Resize(width, height);
    }

    if (combinedPass_)
    {
        combinedPass_->Resize(width, height);
    }

    if (blurHorizontalPass_)
    {
        blurHorizontalPass_->Resize(width, height);
    }

    if (blurVerticalPass_)
    {
        blurVerticalPass_->Resize(width, height);
    }

    if (depthOfFieldPass_)
    {
        depthOfFieldPass_->Resize(width, height);
    }
    
    if (iblPass_)
    {
        iblPass_->Resize(width, height);
    }

    if (postPass_)
    {
        postPass_->Resize(width, height);
    }

    if (screenPass_)
    {
        screenPass_->Resize(width, height);
    }

    if (ssrPass_)
    {
        ssrPass_->Resize(width, height);
    }

    if (oitPass_)
    {
        oitPass_->Resize(width, height);
    }

    // 更新主相机的投影矩阵，以适应新的屏幕宽高比
    sceneData_->camera->setAspectRatio(static_cast<float>(width) / height);
    // camera_.updateProjectionMatrix();
}