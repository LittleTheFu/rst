#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <vector>
#include <Eigen/Dense>
#include <memory> // For std::unique_ptr, std::shared_ptr

// 核心抽象接口
// 不再直接包含 Renderable.h 和 Transformable.h，因为它们现在包含在 ISceneObject.h 中
#include "sceneObject.h" // 现在直接包含 ISceneObject

// 具体实现类
// #include "mesh.h" // Mesh 类不再直接在这里管理，而是由 Model 内部管理
#include "model.h"        // Model 类会继承 ISceneObject
#include "pointLight.h"
#include "camera.h"
#include "textureCubeMap.h"
#include "texture2D.h"

// ObjectData 结构体现在可以完全移除了，因为 ISceneObject 已经包含了所有必要功能
// struct ObjectData {
//     IRenderable* renderable; 
//     ITransformable* transformable;
// };

struct SceneData {
    int screenWidth;
    int screenHeight;

    // --- 关键改动：使用 ISceneObject 统一管理场景中的可渲染、可变换对象 ---
    // opaqueObjects 和 transparentObjects 现在存储的是 Model 或其他 ISceneObject 的实例
    std::vector<std::unique_ptr<ISceneObject>> opaqueObjects;
    std::vector<std::unique_ptr<ISceneObject>> transparentObjects;

    // cursor 和 skybox 如果是可渲染、可变换、可命名的，也应该使用 ISceneObject
    // 如果它们是简单的几何体，可以考虑创建独立的 Mesh 并包装成 ISceneObject
    // 或者直接加载为 Model
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

#endif // SCENEDATA_H