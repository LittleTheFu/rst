#ifndef LIGHTPASS_H
#define LIGHTPASS_H

#include "RenderPass.h"
#include "Camera.h"
#include <Eigen/Dense>

struct SceneData;

class LightPass : public RenderPass
{
public:
    LightPass(const std::string& name);
    virtual ~LightPass() = default;

    virtual void Initialize(int width, int height) override;
    virtual void Render(SceneData& sceneData, Camera& camera) override;
    virtual void Resize(int width, int height) override;

private:
    GLuint quadVAO_;
    GLuint quadVBO_;

    void renderQuad();
};

#endif // LIGHTPASS_H