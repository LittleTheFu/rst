#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include <memory>
#include <vector>

// 核心组件
#include "camera.h"
#include "pointLight.h"
// #include "mesh.h" // Mesh 类不再直接在这里管理，因为它现在是 Model 的一部分
#include "material.h" // 如果材质在Scene中创建
#include "sceneObject.h" // 引入 ISceneObject 接口
// 如果你有其他作为 ISceneObject 的类型，也需要在这里包含它们的头文件
// 例如，如果 Model.h 尚未被其他包含，这里也需要
#include "model.h"

// 纹理类
#include "TextureCubeMap.h"
#include "Texture2D.h"
#include "texture.h" // 通用LDR纹理

// 所有的渲染Pass
#include "gBufferPass.h"
#include "lightPass.h"
#include "skyPass.h"
#include "shadowPass.h"
#include "IBLPass.h"
#include "OitPass.h"
#include "combinedPass.h"
#include "ssrPass.h"
#include "blurHorizontalPass.h"
#include "blurVerticalPass.h"
#include "postPass.h"
#include "screenPass.h"
#include "brightnessMaskPass.h"
#include "godRayPass.h"

#include "SceneData.h"
#include "depthOfFieldPass.h"

#include "debugRenderer.h"
#include "objectPicker.h" // ObjectPicker 现在返回 ISceneObject*

class Scene
{
public:
    void saveTextures();

public:
    void setRenderFlag(int flag) { flag_ = flag; };
    int flag_ = 0;

    void setSsrWeight(float weight) { ssrWeight_ = weight; };
    float ssrWeight_ = 0.2f;

    void setIblWeight(float weight) { iblWeight_ = weight; };
    float iblWeight_ = 0.1f;

    void setLightWeight(float weight) { lightWeight_ = weight; };
    float lightWeight_ = 0.7f;

    void setOitWeight(float weight) { oitWeight_ = weight; };
    float oitWeight_ = 1.0f;

    void setGodRayWeight(float weight) { godRayWeight_ = weight; };
    float godRayWeight_ = 0.1f;

    void setFocusDistance(float distance) { focusDistance_ = distance; };
    float focusDistance_ = 10.0f;

    void toggleDebugDraw() { isDebugDraw_ = !isDebugDraw_; };
    bool isDebugDraw_ = false;

public:
    void init();
    void run();
    void resize(int width, int height); // 处理窗口尺寸变化

private:
    void updateScene();
    void renderFinalPass();
    void debugDraw();

public:
    // 将 setSelectedMesh 和 getSelectedMesh 更改为 ISceneObject
    void setSelectedObject(ISceneObject* obj) { selectedObject_ = obj; };
    ISceneObject* getSelectedObject() { return selectedObject_; };
    
    std::string getSelectedObjectName() const
    {
        if (selectedObject_)
        {
            return selectedObject_->getName(); // 假设 ISceneObject 有 getName() 方法
        }
        return "None Selected"; // 没有物体选中时的显示
    }

    // 修改 getAllMeshes 为 getAllSceneObjects
    std::vector<ISceneObject*> getAllSceneObjects() const;
    PointLight* getPointLight() const { return sceneData_->light.get(); };
    std::shared_ptr<Camera> getCamera() { return sceneData_->camera; };

private:
    ISceneObject* selectedObject_ = nullptr; // 更改为 ISceneObject*

private:
    std::vector<ISceneObject *> getRawPointers(const std::vector<std::unique_ptr<ISceneObject>> &uniquePtrVector)
    {
        std::vector<ISceneObject *> rawPointers;
        rawPointers.reserve(uniquePtrVector.size()); // 预留空间以避免多次重新分配
        for (const auto &ptr : uniquePtrVector)
        {
            if (ptr)
            {
                rawPointers.push_back(ptr.get());
            }
        }
        return rawPointers;
    }

private:
    std::unique_ptr<SceneData> sceneData_; 

    // 渲染 Pass 实例
    std::unique_ptr<SkyPass> skyPass_;
    std::unique_ptr<ShadowPass> shadowPass_;
    std::unique_ptr<OitPass> oitPass_;

    std::unique_ptr<GBufferPass> gBufferPass_;
    std::unique_ptr<LightPass> lightPass_;
    std::unique_ptr<IBLPass> iblPass_;

    std::unique_ptr<BrightnessMaskPass> brightnessMaskPass_;
    std::unique_ptr<GodRayPass> godRayPass_;

    std::unique_ptr<CombinedPass> combinedPass_;
    std::unique_ptr<SSRPass> ssrPass_;
    
    std::unique_ptr<BlurHorizontalPass> blurHorizontalPass_;
    std::unique_ptr<BlurVerticalPass> blurVerticalPass_;
    std::unique_ptr<DepthOfFieldPass> depthOfFieldPass_;

    std::unique_ptr<PostPass> postPass_;
    std::unique_ptr<ScreenPass> screenPass_;

    std::unique_ptr<DebugRenderer> debugRenderer_;

public: // Keep public for now as per your original code
    std::unique_ptr<ObjectPicker> objectPicker_;
};

#endif // _SCENE_H_