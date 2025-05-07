#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <vector>
#include <Eigen/Dense>
#include "Renderable.h" // 使用 Renderable 接口
#include "transformAble.h" // 使用 Transformable 接口


struct ObjectData {
    Renderable* renderable; // 指向可渲染对象的指针
    Transformable* transformable; // 指向可变换对象的指针
    // Material material; 
};

struct SceneData {
    int screenWidth;
    int screenHeight;
    std::vector<ObjectData> objects; // 场景中的所有物体
    // 可以添加其他场景全局数据，例如：
    // Light light;
};

#endif // SCENEDATA_H