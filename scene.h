#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include <memory>
#include <vector>

// 核心组件
#include "camera.h"
#include "pointLight.h"
#include "mesh.h"
#include "material.h" // 如果材质在Scene中创建

// 纹理类
#include "TextureCubeMap.h"
#include "Texture2D.h"
#include "texture.h" // 通用LDR纹理

// 所有的渲染Pass
#include "GBufferPass.h"
#include "lightPass.h"
#include "screenPass.h"
#include "skyPass.h"
#include "shadowPass.h"
#include "IBLPass.h"
#include "OitPass.h"

// SceneData 现在可以更精简，只包含全局共享的固定信息，或者直接移除，
// 因为大部分动态数据现在通过Render函数传递。
// 为保持兼容性，我们暂时保留它，但可以按需精简。
#include "SceneData.h"

class Scene
{
public:
    void init();
    void run();
    void resize(int width, int height); // 处理窗口尺寸变化

    // 可以添加获取相机或灯光的函数，以便外部（如ImGui）控制
    Camera& getCamera() { return camera_; }
    PointLight& getMainLight() { return *mainLight_; }

private:
    SceneData sceneData_; // 暂时保留，根据实际需求决定是否完全移除或精简

    // 场景中的核心对象
    Camera camera_;
    Camera shadow_camera_; // 用于阴影渲染的相机
    std::shared_ptr<PointLight> mainLight_; // 主光源
    std::vector<std::unique_ptr<Mesh>> meshes_; // 场景中的所有可渲染网格

    // 渲染 Pass 实例
    std::unique_ptr<GBufferPass> gBufferPass_;
    std::unique_ptr<LightPass> lightPass_;
    // std::unique_ptr<SkyPass> skyPass_;
    std::unique_ptr<ScreenPass> screenPass_;
    std::unique_ptr<ShadowPass> shadowPass_;
    std::unique_ptr<IBLPass> iblPass_;
    std::unique_ptr<OitPass> oitPass_;

    // IBL 预计算纹理，由 Scene 管理其生命周期并注入给 IBLPass 和 SkyPass
    std::shared_ptr<TextureCubeMap> irradianceMapTex_;
    std::shared_ptr<TextureCubeMap> prefilterMapTex_;
    std::shared_ptr<Texture2D> brdfLUTTex_;
};

#endif // _SCENE_H_