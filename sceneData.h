#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <vector>
#include <Eigen/Dense>
#include "Mesh.h" // 假设你有一个 Mesh 类来表示网格数据

struct ObjectData {
    Eigen::Matrix4f modelMatrix; // 物体的模型矩阵
    Mesh* mesh;             // 指向物体网格数据的指针
    // 可以添加其他材质相关的数据，例如：
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