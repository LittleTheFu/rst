#include "ObjectPicker.h"
#include <limits>    // For std::numeric_limits
#include <iostream>  // For debugging output (picked mesh info)
#include <iomanip>   // For std::hex, std::setw for address printing

ObjectPicker::ObjectPicker(SceneData* sceneData, Camera* camera)
    : sceneData_(sceneData), camera_(camera) {}

// 将 screenPointToWorldRay 标记为 const，因为它不修改 ObjectPicker 的成员
Ray ObjectPicker::screenPointToWorldRay(int mouseX, int mouseY) const {
    // 1. 屏幕坐标到 NDC
    // 注意：sceneData_ 是指针，需要使用 -> 访问其成员
    float ndcX = (2.0f * mouseX) / sceneData_->screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / sceneData_->screenHeight; // Y轴翻转
    float ndcZ = 0.0f; // 设为0，通常表示近裁剪平面，用于射线起点

    Eigen::Vector4f clipCoords(ndcX, ndcY, ndcZ, 1.0f);

    // 2. NDC 到眼空间 (视图空间)
    // 注意：camera_ 是 unique_ptr，需要使用 .get() 获取原始指针，然后使用 -> 访问其方法
    Eigen::Matrix4f inverseProjectionMatrix = camera_->GetProjectionMatrix().inverse();
    Eigen::Vector4f eyeCoords = inverseProjectionMatrix * clipCoords;
    
    // 对于射线方向，我们通常将眼空间 Z 设为 -1.0f (指向屏幕内部的远裁剪平面)
    // 并且 W 设为 0.0f，以表示这是一个方向向量，而不是一个点
    Eigen::Vector4f eyeRayDir(eyeCoords.x(), eyeCoords.y(), -1.0f, 0.0f); 

    // 3. 眼空间到世界空间
    Eigen::Matrix4f inverseViewMatrix = camera_->GetViewMatrix().inverse();
    Eigen::Vector4f worldRayDirHomogeneous = inverseViewMatrix * eyeRayDir;

    // 4. 创建 Ray 对象
    Eigen::Vector3f rayOrigin = camera_->getPosition(); // 摄像机在世界空间的位置是射线的起点
    Eigen::Vector3f rayDirection = worldRayDirHomogeneous.head<3>().normalized(); // 归一化方向向量
    
    return Ray(rayOrigin, rayDirection);
}

Mesh* ObjectPicker::pick(int mouseX, int mouseY) {
    Ray pickingRay = screenPointToWorldRay(mouseX, mouseY);

    float closestT = std::numeric_limits<float>::max();
    Mesh* pickedMesh = nullptr;

    // 遍历不透明物体
    // 注意：opaqueObjects 是 std::vector<std::unique_ptr<Mesh>>
    // 所以在循环中，mesh 是 std::unique_ptr<Mesh> 的引用
    // 需要通过 mesh.get() 获取原始 Mesh* 指针
    for (const auto& meshPtr : sceneData_->opaqueObjects) {
        if (meshPtr) { // 确保 unique_ptr 不为空
            Mesh* mesh = meshPtr.get(); // 获取原始 Mesh*
            AABB* worldAABB = mesh->getWorldAABB(); // 获取世界空间 AABB
            
            float t_min_val, t_max_val; // Ray::intersectsAABB 需要这两个参数
            if (pickingRay.intersectsAABB(*worldAABB, t_min_val, t_max_val)) {
                // 我们只关心射线前方（t >= 0）且是最近的交点
                // 如果射线原点在 AABB 内部，t_min_val 可能为负，但仍应被视为有效交点（从原点开始算距离为0）
                float currentT = (t_min_val < 0.0f) ? 0.0f : t_min_val;

                if (currentT < closestT) {
                    closestT = currentT;
                    pickedMesh = mesh;
                }
            }
            delete worldAABB; // 释放 getWorldAABB 返回的堆内存，非常重要
        }
    }

    // 遍历透明物体 (可选，取决于你的需求，通常透明物体在拾取时优先级较低或不拾取)
    // 如果你希望透明物体也能被拾取，且优先级低于不透明物体，可以保留此循环
    // 如果希望透明物体也能拾取且可能有更高优先级（比如UI元素），则需要调整逻辑
    for (const auto& meshPtr : sceneData_->transparentObjects) {
        if (meshPtr) { // 确保 unique_ptr 不为空
            Mesh* mesh = meshPtr.get(); // 获取原始 Mesh*
            AABB* worldAABB = mesh->getWorldAABB();
            
            float t_min_val, t_max_val;
            if (pickingRay.intersectsAABB(*worldAABB, t_min_val, t_max_val)) {
                float currentT = (t_min_val < 0.0f) ? 0.0f : t_min_val;
                
                if (currentT < closestT) { // 仍然找最近的
                    closestT = currentT;
                    pickedMesh = mesh;
                }
            }
            delete worldAABB; // 释放堆内存
        }
    }

    return pickedMesh;
}