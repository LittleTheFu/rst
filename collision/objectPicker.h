#ifndef OBJECT_PICKER_H
#define OBJECT_PICKER_H

#include <Eigen/Dense>
#include <vector>
#include <memory>
#include "Ray.h"
// #include "Mesh.h"      // 不再直接需要 Mesh.h，因为我们现在与 ISceneObject 交互
#include "sceneObject.h" // 包含 ISceneObject 接口
#include "SceneData.h"   // 你的 SceneData 结构体
#include "Camera.h"
#include "BoundingVolume.h"

class ObjectPicker
{
public:
    ObjectPicker(SceneData *sceneData, Camera *camera);

    ISceneObject *pick(int mouseX, int mouseY); // 返回 ISceneObject*

private:
    SceneData *sceneData_;
    Camera *camera_;

    Ray screenPointToWorldRay(int mouseX, int mouseY) const;
};

#endif