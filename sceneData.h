#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <vector>
#include <Eigen/Dense>
#include "Renderable.h"
#include "transformAble.h"
#include "mesh.h"
#include "pointLight.h"
#include "camera.h"


struct ObjectData {
    Renderable* renderable; // 指向可渲染对象的指针
    Transformable* transformable; // 指向可变换对象的指针
};

struct SceneData {
    int screenWidth;
    int screenHeight;

    std::vector<std::unique_ptr<Mesh>> opaqueObjects;
    std::vector<std::unique_ptr<Mesh>> transparentObjects;
    std::unique_ptr<Mesh> cursor;
    std::unique_ptr<Mesh> skybox;
    std::unique_ptr<PointLight> light;
    std::unique_ptr<Camera> camera;

    int shadowMapWidth;
    int shadowMapHeight;
};

class sceneFactory {
public:
    static std::unique_ptr<SceneData> createScene(); 
};

#endif // SCENEDATA_H