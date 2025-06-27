#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <vector>
#include <Eigen/Dense>
#include "Renderable.h"
#include "transformAble.h"
#include "mesh.h"
#include "pointLight.h"
#include "camera.h"
#include "textureCubeMap.h"
#include "texture2D.h"



struct ObjectData {
    IRenderable* renderable; // 指向可渲染对象的指针
    ITransformable* transformable; // 指向可变换对象的指针
};

struct SceneData {
    int screenWidth;
    int screenHeight;

    std::vector<std::unique_ptr<Mesh>> opaqueObjects;
    std::vector<std::unique_ptr<Mesh>> transparentObjects;
    std::unique_ptr<Mesh> cursor;
    std::unique_ptr<Mesh> skybox;
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

#endif // SCENEDATA_H