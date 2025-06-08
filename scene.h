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

class Scene
{
public:
    void blur(bool isOn);
    bool isBlurOn_ = true;

public:
    void setRenderFlag(int flag) { flag_ = flag; };
    int flag_ = 0;

public:
    void setSsrWeight(float weight) { ssrWeight_ = weight; };
    float ssrWeight_ = 0.2f;

    void setIblWeight(float weight) { iblWeight_ = weight; };
    float iblWeight_ = 0.1f;

    void setLightWeight(float weight) { lightWeight_ = weight; };
    float lightWeight_ = 0.7f;

    void setOitWeight(float weight) { oitWeight_ = weight; };
    float oitWeight_ = 1.0f;

public:
    void init();
    void run();
    void resize(int width, int height); // 处理窗口尺寸变化

private:
    void updateScene();

private:
    std::unique_ptr<SceneData> sceneData_; // 暂时保留，根据实际需求决定是否完全移除或精简

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
};

#endif // _SCENE_H_