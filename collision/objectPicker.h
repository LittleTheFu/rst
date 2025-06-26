#ifndef OBJECT_PICKER_H
#define OBJECT_PICKER_H

#include <Eigen/Dense>
#include <vector>
#include <memory>      // For std::unique_ptr
#include "Ray.h"       // 你的 Ray 类
#include "Mesh.h"      // 你的 Mesh 类
#include "SceneData.h" // 你的 SceneData 结构体
#include "Camera.h"    // 你的 Camera 类
#include "BoundingVolume.h" // 包含 AABB 的定义

class ObjectPicker {
public:
    /**
     * @brief 构造函数。
     * @param sceneData 场景数据的指针。
     * @param camera 当前场景的相机指针。
     * 注意：这里接受指针是为了方便访问，SceneData和Camera的生命周期由外部管理。
     */
    ObjectPicker(SceneData* sceneData, Camera* camera);

    /**
     * @brief 执行物体拾取。
     * @param mouseX 鼠标 X 坐标（屏幕空间）。
     * @param mouseY 鼠标 Y 坐标（屏幕空间）。
     * @return 指向被拾取 Mesh 的指针，如果没有拾取到任何物体，则返回 nullptr。
     * 注意：返回的是原始指针，所有权仍在 SceneData 中。
     */
    Mesh* pick(int mouseX, int mouseY);

private:
    SceneData* sceneData_;
    Camera* camera_;

    /**
     * @brief 将屏幕坐标转换为世界空间射线。
     * @param mouseX 鼠标 X 坐标（屏幕空间）。
     * @param mouseY 鼠标 Y 坐标（屏幕空间）。
     * @return 世界空间射线。
     */
    Ray screenPointToWorldRay(int mouseX, int mouseY) const; // 标记为 const
};

#endif