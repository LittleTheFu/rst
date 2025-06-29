#include "scene.h"
#include <iostream>
#include <glad/glad.h>
#include "debug_utils.h"
#include "utilities.h"
#include "model.h"

// Jolt-specific includes
#include <Jolt/Physics/PhysicsSystem.h> // Required for JPH::DebugRenderer::sInstance
// Make sure this is "joltDebugRenderer.h" as you renamed it
#include "joltDebugRenderer.h" // Your custom Jolt debug renderer

// (Assuming your Scene.h has been updated as per the previous response
// to change debugRenderer_ type to std::unique_ptr<JoltDebugRenderer>)

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

    brightnessMaskPass_ = std::make_unique<BrightnessMaskPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    godRayPass_ = std::make_unique<GodRayPass>(sceneData_->screenWidth, sceneData_->screenHeight);

    combinedPass_ = std::make_unique<CombinedPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    ssrPass_ = std::make_unique<SSRPass>(sceneData_->screenWidth, sceneData_->screenHeight);

    blurHorizontalPass_ = std::make_unique<BlurHorizontalPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    blurVerticalPass_ = std::make_unique<BlurVerticalPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    depthOfFieldPass_ = std::make_unique<DepthOfFieldPass>(sceneData_->screenWidth, sceneData_->screenHeight);

    postPass_ = std::make_unique<PostPass>(sceneData_->screenWidth, sceneData_->screenHeight);
    screenPass_ = std::make_unique<ScreenPass>(sceneData_->screenWidth, sceneData_->screenHeight);

    // --- JoltDebugRenderer Initialization ---
    // Make sure debugRenderer_ is of type std::unique_ptr<JoltDebugRenderer> in Scene.h
    debugRenderer_ = std::make_unique<JoltDebugRenderer>();
    debugRenderer_->Init(); // Call Init for JoltDebugRenderer
    GL_CHECK_ERROR();

    // !!! CRUCIAL STEP: Register your JoltDebugRenderer with Jolt's global instance !!!
    JPH::DebugRenderer::sInstance = debugRenderer_.get();
    std::cout << "JoltDebugRenderer registered with Jolt's global instance." << std::endl;

    objectPicker_ = std::make_unique<ObjectPicker>(sceneData_.get(), sceneData_->camera.get());

    physicsSystem_ = std::make_unique<PhysicsSystem>();
    physicsSystem_->Init();

    for (const auto &objPtr : sceneData_->opaqueObjects)
    {
        if (objPtr)
        {
            if (objPtr->getName().find("teapot") != std::string::npos)
            {
                physicsSystem_->AddSceneObject(objPtr.get(), JPH::EMotionType::Dynamic, Layers::Object::MOVING);
            }
            else
            {
                physicsSystem_->AddSceneObject(objPtr.get(), JPH::EMotionType::Static, Layers::Object::NON_MOVING);
            }
        }
    }

    for (const auto &objPtr : sceneData_->transparentObjects)
    {
        if (objPtr)
        {
            physicsSystem_->AddSceneObject(objPtr.get(), JPH::EMotionType::Dynamic, Layers::Object::MOVING);
        }
    }
}

void Scene::updateScene(float delta)
{
    physicsSystem_->Update(delta); // Pass deltaTime to the physics system for simulation

    static int count = 0;
    count++;
    count %= 8000;
    float x_light = count / 1200.0f - 3.0f;
    x_light *= 1.0f;
    sceneData_->light->position = Eigen::Vector3f(x_light, x_light, 3.0f);

    // No need for cursor update if it's just for light position visualization
}

void Scene::renderFinalPass()
{
    if (flag_ == 0)
    {
        screenPass_->Render(postPass_->getColorTextureId());
    }
    else if (flag_ == 1)
    {
        screenPass_->Render(gBufferPass_->getPositionTextureId());
    }
    else if (flag_ == 2)
    {
        screenPass_->Render(gBufferPass_->getNormalTextureId());
    }
    else if (flag_ == 3)
    {
        screenPass_->Render(gBufferPass_->getAlbedoTextureId());
    }
    else if (flag_ == 4)
    {
        screenPass_->Render(gBufferPass_->getRoughnessTextureId());
    }
    else if (flag_ == 5)
    {
        screenPass_->Render(gBufferPass_->getMetallicTextureId());
    }
    else if (flag_ == 6)
    {
        screenPass_->Render(gBufferPass_->getAOTextureId());
    }
    else if (flag_ == 7)
    {
        screenPass_->Render(lightPass_->getOutputTextureId());
    }
    else if (flag_ == 8)
    {
        screenPass_->Render(iblPass_->getColorTextureId());
    }
    else if (flag_ == 9)
    {
        screenPass_->Render(combinedPass_->getColorTextureId());
    }
    else if (flag_ == 10)
    {
        screenPass_->Render(ssrPass_->getReflectionTextureId());
    }
    else if (flag_ == 11)
    {
        screenPass_->Render(lightPass_->getDebugCurrentDepthTextureId());
    }
    else if (flag_ == 12)
    {
        screenPass_->Render(lightPass_->getDebugClosestDepthTextureId());
    }
    else if (flag_ == 13)
    {
        screenPass_->Render(brightnessMaskPass_->getOutputTextureId());
    }
    else if (flag_ == 14)
    {
        screenPass_->Render(godRayPass_->getColorTextureId());
    }

    GL_CHECK_ERROR();
}

// --- Modified debugDraw() ---
void Scene::debugDraw()
{
    // Bind to default framebuffer to draw directly to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, sceneData_->screenWidth, sceneData_->screenHeight);

    // Clear any accumulated debug data from the previous frame
    debugRenderer_->Clear();

    // Tell the PhysicsSystem to draw its debug information.
    // This will internally call JoltDebugRenderer's DrawTriangle/DrawLine methods
    // via JPH::DebugRenderer::sInstance.
    physicsSystem_->DrawDebug();

    // Finally, flush (render) all accumulated debug data using the camera's matrices.
    debugRenderer_->Flush(sceneData_->camera->GetViewMatrix(), sceneData_->camera->GetProjectionMatrix());

    GL_CHECK_ERROR();
}

std::vector<ISceneObject *> Scene::getAllSceneObjects() const
{
    std::vector<ISceneObject *> allObjects;
    allObjects.reserve(sceneData_->opaqueObjects.size() +
                       sceneData_->transparentObjects.size() +
                       (sceneData_->cursor ? 1 : 0) +
                       (sceneData_->skybox ? 1 : 0));

    for (const auto &objPtr : sceneData_->opaqueObjects)
    {
        if (objPtr)
        {
            allObjects.push_back(objPtr.get());
        }
    }

    for (const auto &objPtr : sceneData_->transparentObjects)
    {
        if (objPtr)
        {
            allObjects.push_back(objPtr.get());
        }
    }

    if (sceneData_->cursor)
    {
        allObjects.push_back(sceneData_->cursor.get());
    }

    if (sceneData_->skybox)
    {
        allObjects.push_back(sceneData_->skybox.get());
    }

    return allObjects;
}

void Scene::run(float delta)
{
    updateScene(delta);

    skyPass_->Render(*sceneData_->camera);
    GL_CHECK_ERROR();

    std::vector<ISceneObject *> rawopaqueObjects = getRawPointers(sceneData_->opaqueObjects);
    if (sceneData_->cursor)
    {
        rawopaqueObjects.push_back(sceneData_->cursor.get());
    }

    shadowPass_->Render(rawopaqueObjects, *sceneData_->light);
    GL_CHECK_ERROR();

    gBufferPass_->Render(rawopaqueObjects, *sceneData_->camera);
    GL_CHECK_ERROR();

    oitPass_->Render(getRawPointers(sceneData_->transparentObjects),
                     *sceneData_->light,
                     *sceneData_->camera,
                     gBufferPass_->getDepthTextureId());
    GL_CHECK_ERROR();

    lightPass_->Render(gBufferPass_->getPositionTextureId(),
                       gBufferPass_->getNormalTextureId(),
                       gBufferPass_->getAlbedoTextureId(),
                       gBufferPass_->getRoughnessTextureId(),
                       gBufferPass_->getMetallicTextureId(),
                       gBufferPass_->getAOTextureId(),
                       *sceneData_->light,
                       *sceneData_->camera,
                       shadowPass_->getShadowMapDepthOutputTextureId());
    GL_CHECK_ERROR();

    brightnessMaskPass_->Render(lightPass_->getOutputTextureId());
    GL_CHECK_ERROR();

    godRayPass_->Render(brightnessMaskPass_->getOutputTextureId(),
                        sceneData_->light->position,
                        sceneData_->camera->GetViewMatrix(),
                        sceneData_->camera->GetProjectionMatrix(),
                        1.0f,
                        0.95f,
                        0.8f,
                        0.3f,
                        32);
    GL_CHECK_ERROR();

    iblPass_->Render(gBufferPass_->getPositionTextureId(),
                     gBufferPass_->getNormalTextureId(),
                     gBufferPass_->getAlbedoTextureId(),
                     gBufferPass_->getRoughnessTextureId(),
                     gBufferPass_->getMetallicTextureId(),
                     gBufferPass_->getAOTextureId(),
                     *sceneData_->camera);
    GL_CHECK_ERROR();

    ssrPass_->Render(gBufferPass_->getNormalTextureId(),
                     gBufferPass_->getDepthTextureId(),
                     gBufferPass_->getAlbedoTextureId(),
                     gBufferPass_->getMetallicTextureId(),
                     gBufferPass_->getRoughnessTextureId(),
                     sceneData_->camera->GetProjectionMatrix(),
                     sceneData_->camera->GetViewMatrix());
    GL_CHECK_ERROR();

    combinedPass_->Render(lightPass_->getOutputTextureId(),
                          iblPass_->getColorTextureId(),
                          gBufferPass_->getDepthTextureId(),
                          oitPass_->getAccumTextureId(),
                          oitPass_->getRevealTextureId(),
                          skyPass_->getColorTextureId(),
                          ssrPass_->getReflectionTextureId(),
                          godRayPass_->getColorTextureId(),
                          ssrWeight_,
                          iblWeight_,
                          lightWeight_,
                          oitWeight_,
                          godRayWeight_);
    GL_CHECK_ERROR();

    blurHorizontalPass_->Render(combinedPass_->getColorTextureId());
    GL_CHECK_ERROR();

    blurVerticalPass_->Render(blurHorizontalPass_->getColorTextureId());
    GL_CHECK_ERROR();

    depthOfFieldPass_->Render(combinedPass_->getColorTextureId(),
                              blurVerticalPass_->getColorTextureId(),
                              gBufferPass_->getDepthTextureId(),
                              focusDistance_,
                              4.0f,
                              sceneData_->camera->nearClip,
                              sceneData_->camera->farClip);

    postPass_->Render(depthOfFieldPass_->getColorTextureId());
    GL_CHECK_ERROR();

    renderFinalPass(); // Renders the final scene to the screen

    // --- Debug Draw after all other rendering ---
    // This ensures debug lines/shapes are always visible on top
    if (isDebugDraw_ && debugRenderer_ && physicsSystem_)
    {
        debugDraw(); // Call the dedicated debug drawing function
    }
}

void Scene::resize(int width, int height)
{
    sceneData_->screenWidth = width;
    sceneData_->screenHeight = height;

    if (gBufferPass_) gBufferPass_->Resize(width, height);
    if (lightPass_) lightPass_->Resize(width, height);
    if (skyPass_) skyPass_->Resize(width, height);
    if (combinedPass_) combinedPass_->Resize(width, height);
    if (blurHorizontalPass_) blurHorizontalPass_->Resize(width, height);
    if (blurVerticalPass_) blurVerticalPass_->Resize(width, height);
    if (depthOfFieldPass_) depthOfFieldPass_->Resize(width, height);
    if (iblPass_) iblPass_->Resize(width, height);
    if (postPass_) postPass_->Resize(width, height);
    if (screenPass_) screenPass_->Resize(width, height);
    if (ssrPass_) ssrPass_->Resize(width, height);
    if (brightnessMaskPass_) brightnessMaskPass_->Resize(width, height);
    if (godRayPass_) godRayPass_->Resize(width, height);
    if (oitPass_) oitPass_->Resize(width, height);

    sceneData_->camera->setAspectRatio(static_cast<float>(width) / height);
    // JoltDebugRenderer usually doesn't need a resize call as it uses the camera's matrices directly.
}

void Scene::saveTextures()
{
    int w = sceneData_->screenWidth;
    int h = sceneData_->screenHeight;

    Utilities::SaveTextureToFile(gBufferPass_->getPositionTextureId(), w, h, GL_RGBA, GL_FLOAT, "position.png");
    Utilities::SaveTextureToFile(gBufferPass_->getNormalTextureId(), w, h, GL_RGBA, GL_FLOAT, "normal.png", true);
    Utilities::SaveTextureToFile(gBufferPass_->getAlbedoTextureId(), w, h, GL_RGBA, GL_UNSIGNED_BYTE, "albedo.png");
    Utilities::SaveTextureToFile(gBufferPass_->getRoughnessTextureId(), w, h, GL_RGBA, GL_UNSIGNED_BYTE, "roughness.png");
    Utilities::SaveTextureToFile(gBufferPass_->getMetallicTextureId(), w, h, GL_RGBA, GL_UNSIGNED_BYTE, "metallic.png");
    Utilities::SaveTextureToFile(gBufferPass_->getAOTextureId(), w, h, GL_RGBA, GL_UNSIGNED_BYTE, "ao.png");
    Utilities::SaveTextureToFile(gBufferPass_->getDepthTextureId(), w, h, GL_DEPTH_COMPONENT, GL_FLOAT, "depth.png");
    Utilities::SaveTextureToFile(skyPass_->getColorTextureId(), w, h, GL_RGBA, GL_UNSIGNED_BYTE, "skybox.png");
    Utilities::SaveTextureToFile(lightPass_->getOutputTextureId(), w, h, GL_RGBA, GL_FLOAT, "light.png");
    Utilities::SaveTextureToFile(combinedPass_->getColorTextureId(), w, h, GL_RGBA, GL_FLOAT, "combined.png");
    Utilities::SaveTextureToFile(brightnessMaskPass_->getOutputTextureId(), w, h, GL_RGBA, GL_UNSIGNED_BYTE, "brightness_mask.png");
    Utilities::SaveTextureToFile(godRayPass_->getColorTextureId(), w, h, GL_RGBA, GL_FLOAT, "god_ray.png");
    Utilities::SaveTextureToFile(ssrPass_->getReflectionTextureId(), w, h, GL_RGBA, GL_FLOAT, "ssr.png");
    Utilities::SaveTextureToFile(blurHorizontalPass_->getColorTextureId(), w, h, GL_RGBA, GL_FLOAT, "blur_horizontal.png");
    Utilities::SaveTextureToFile(blurVerticalPass_->getColorTextureId(), w, h, GL_RGBA, GL_FLOAT, "blur_vertical.png");
    Utilities::SaveTextureToFile(depthOfFieldPass_->getColorTextureId(), w, h, GL_RGBA, GL_FLOAT, "depth_of_field.png");
    Utilities::SaveTextureToFile(postPass_->getColorTextureId(), w, h, GL_RGBA, GL_FLOAT, "post.png");
}