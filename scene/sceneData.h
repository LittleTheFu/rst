#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <vector>
#include <Eigen/Dense>
#include <memory>

#include "sceneObject.h"
#include "model.h"
#include "pointLight.h"
#include "camera.h"
#include "textureCubeMap.h"
#include "texture2D.h"

struct SceneData {
    int screenWidth;
    int screenHeight;

    std::vector<std::unique_ptr<ISceneObject>> opaqueObjects;
    std::vector<std::unique_ptr<ISceneObject>> transparentObjects;

    std::unique_ptr<ISceneObject> cursor;
    std::unique_ptr<ISceneObject> skybox;

    std::unique_ptr<PointLight> light;
    std::shared_ptr<Camera> camera;

    std::shared_ptr<TextureCubeMap> irradianceMapTex_;
    std::shared_ptr<TextureCubeMap> prefilterMapTex_;
    std::shared_ptr<Texture2D> brdfLUTTex_;

    int shadowMapWidth;
    int shadowMapHeight;
};

class sceneFactory {
public:
    static std::unique_ptr<SceneData> createScene(); 
};

#endif
