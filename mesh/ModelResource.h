#ifndef MODEL_RESOURCE_H
#define MODEL_RESOURCE_H

#include <string>
#include <vector>
#include <Eigen/Dense> // For Eigen::Matrix4f

/**
 * @brief ModelMeshInfo 结构体表示模型中的一个可绘制子网格。
 * 包含对 MeshGLData 和 Material 的引用，以及该子网格的局部变换。
 */
struct ModelMeshInfo {
    unsigned int meshId;     // MeshManager 中 MeshGLData 的 ID
    std::string materialName; // MaterialManager 中 Material 的名称 (ID)
                              // 注意：这里使用 materialName 是因为 MaterialManager 接受名称作为 ID
    Eigen::Matrix4f localTransform; // 该网格相对于其父节点或模型根的局部变换

    ModelMeshInfo() : meshId(0), localTransform(Eigen::Matrix4f::Identity()) {}
};

/**
 * @brief ModelResource 类表示一个已加载的 3D 模型的结构。
 * 它不包含 OpenGL 渲染细节，而是引用 MeshGLData 和 Material。
 */
class ModelResource {
public:
    std::string name; // 模型的名称，也作为 ModelManager 中的唯一 ID
    std::vector<ModelMeshInfo> meshes; // 模型包含的所有子网格列表

    // 构造函数
    ModelResource(const std::string& modelName = "default_model") : name(modelName) {}

    // 如果将来支持节点层次和动画，可以在这里添加：
    // struct Node {
    //     std::string name;
    //     Eigen::Matrix4f transformation; // 节点自身的局部变换
    //     std::vector<unsigned int> meshIndices; // 指向 ModelResource::meshes 的索引
    //     std::vector<unsigned int> children; // 指向 ModelResource::nodes 的索引
    // };
    // std::vector<Node> nodes;
    // unsigned int rootNodeIndex;
};

#endif // MODEL_RESOURCE_H