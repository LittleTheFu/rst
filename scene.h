#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include "camera.h"
// 包含所有 RenderPass 的头文件
#include "GBufferPass.h"
#include "lightPass.h"
#include "screenPass.h"
#include "skyPass.h"
#include "shadowPass.h"
#include "IBLPass.h"
#include "SceneData.h" // 包含 SceneData
#include <memory>      // 用于 std::unique_ptr 和 std::shared_ptr
#include "mesh.h"      // 包含 Mesh
#include "TextureCubeMap.h" // 包含 TextureCubeMap
#include "Texture2D.h"      // 包含 Texture2D
#include "pointLight.h"     // 包含 PointLight

class Scene
{
public:
    void init();
    void run();
    void resize(int width, int height); // 新增：处理窗口尺寸变化

private:
    SceneData sceneData_;

    // 使用 unique_ptr 管理各个渲染 Pass
    std::unique_ptr<GBufferPass> gBufferPass_;
    std::unique_ptr<LightPass> lightPass_;
    std::unique_ptr<SkyPass> skyPass_;
    std::unique_ptr<ScreenPass> screenPass_;
    std::unique_ptr<ShadowPass> shadowPass_;
    std::unique_ptr<IBLPass> iblPass_;

    // IBL 预计算纹理，由 Scene 管理其生命周期并注入给 IBLPass 和 SkyPass
    std::shared_ptr<TextureCubeMap> irradianceMapTex_;
    std::shared_ptr<TextureCubeMap> prefilterMapTex_;
    std::shared_ptr<Texture2D> brdfLUTTex_;

    // 用于天空盒的原始环境贴图（可选，如果 IBL 不直接用它渲染天空）
    // std::shared_ptr<TextureCubeMap> environmentMapTex_;

    Camera camera_;
    Camera shadow_camera_;

    // 场景中的物体和光源，由 Scene 管理
    std::vector<std::unique_ptr<Mesh>> meshes_; // 替代 sceneData_.objects
    std::shared_ptr<PointLight> mainLight_;     // 替代 sceneData_.light
};

#endif // _SCENE_H_