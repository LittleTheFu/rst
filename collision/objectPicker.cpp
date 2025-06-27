#include "ObjectPicker.h"
#include <limits>
#include <iostream>
#include <iomanip>

// 确保包含 Model.h，以便能对 ISceneObject* 进行 dynamic_cast
// 如果你有很多种 ISceneObject，可以考虑更通用的方式，或者让 ISceneObject 有一个 getBoundingVolume() 方法
#include "Model.h"

ObjectPicker::ObjectPicker(SceneData *sceneData, Camera *camera)
    : sceneData_(sceneData), camera_(camera) {}

Ray ObjectPicker::screenPointToWorldRay(int mouseX, int mouseY) const
{
    float ndcX = (2.0f * mouseX) / sceneData_->screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / sceneData_->screenHeight;
    float ndcZ = 0.0f;

    Eigen::Vector4f clipCoords(ndcX, ndcY, ndcZ, 1.0f);

    Eigen::Matrix4f inverseProjectionMatrix = camera_->GetProjectionMatrix().inverse();
    Eigen::Vector4f eyeCoords = inverseProjectionMatrix * clipCoords;

    Eigen::Vector4f eyeRayDir(eyeCoords.x(), eyeCoords.y(), -1.0f, 0.0f);

    Eigen::Matrix4f inverseViewMatrix = camera_->GetViewMatrix().inverse();
    Eigen::Vector4f worldRayDirHomogeneous = inverseViewMatrix * eyeRayDir;

    Eigen::Vector3f rayOrigin = camera_->getPosition();
    Eigen::Vector3f rayDirection = worldRayDirHomogeneous.head<3>().normalized();

    return Ray(rayOrigin, rayDirection);
}

// 拾取现在返回 ISceneObject*
ISceneObject *ObjectPicker::pick(int mouseX, int mouseY)
{
    Ray pickingRay = screenPointToWorldRay(mouseX, mouseY);

    float closestT = std::numeric_limits<float>::max();
    ISceneObject *pickedObject = nullptr; // 现在拾取的是 ISceneObject

    // 遍历不透明物体 (都是 ISceneObject)
    for (const auto &objPtr : sceneData_->opaqueObjects)
    {
        if (objPtr)
        { // 确保 unique_ptr 不为空
            ISceneObject *currentObj = objPtr.get();

            // 尝试将其转换为 Model*，因为 Model 提供了 getWorldAABB()
            // 如果你有其他 ISceneObject 类型也需要拾取，它们也需要实现类似 getWorldAABB() 的方法
            Model *model = dynamic_cast<Model *>(currentObj);
            if (model)
            {
                std::unique_ptr<AABB> worldAABB = model->getWorldAABB(); // 获取 Model 的世界空间 AABB

                if (worldAABB)
                { // 确保 AABB 存在 (模型可能没有 Mesh)
                    float t_min_val, t_max_val;
                    if (pickingRay.intersectsAABB(*worldAABB, t_min_val, t_max_val))
                    {
                        float currentT = (t_min_val < 0.0f) ? 0.0f : t_min_val;

                        if (currentT < closestT)
                        {
                            closestT = currentT;
                            pickedObject = currentObj;
                        }
                    }
                }
            }
            else
            {
                // 如果 ISceneObject 不是 Model 类型，或者它没有提供 getBoundingVolume()
                // 你需要在这里添加对其他 ISceneObject 类型的处理
                // 例如，如果 ISceneObject 接口本身就有一个 pure virtual getBoundingVolume() 方法
                // virtual std::unique_ptr<AABB> getBoundingVolume() const = 0;
                // 那么这里就可以直接调用 currentObj->getBoundingVolume()
                // 对于简单的 Mesh，如果它也被直接添加到 opaqueObjects，它也应该实现这个方法
            }
        }
    }

    // 遍历透明物体 (可选，取决于你的需求，通常透明物体在拾取时优先级较低或不拾取)
    for (const auto &objPtr : sceneData_->transparentObjects)
    {
        if (objPtr)
        {
            ISceneObject *currentObj = objPtr.get();
            Model *model = dynamic_cast<Model *>(currentObj);
            if (model)
            {
                std::unique_ptr<AABB> worldAABB = model->getWorldAABB();
                if (worldAABB)
                {
                    float t_min_val, t_max_val;
                    if (pickingRay.intersectsAABB(*worldAABB, t_min_val, t_max_val))
                    {
                        float currentT = (t_min_val < 0.0f) ? 0.0f : t_min_val;
                        if (currentT < closestT)
                        {
                            closestT = currentT;
                            pickedObject = currentObj;
                        }
                    }
                }
            }
        }
    }

    // 对 cursor 和 skybox 进行拾取 (如果它们也是 ISceneObject)
    if (sceneData_->cursor)
    {
        Model *cursorModel = dynamic_cast<Model *>(sceneData_->cursor.get());
        if (cursorModel)
        {
            std::unique_ptr<AABB> worldAABB = cursorModel->getWorldAABB();
            if (worldAABB)
            {
                float t_min_val, t_max_val;
                if (pickingRay.intersectsAABB(*worldAABB, t_min_val, t_max_val))
                {
                    float currentT = (t_min_val < 0.0f) ? 0.0f : t_min_val;
                    if (currentT < closestT)
                    { // cursor 应该有高优先级，或者独立拾取逻辑
                        closestT = currentT;
                        pickedObject = sceneData_->cursor.get();
                    }
                }
            }
        }
    }

    // skybox 通常不参与拾取，因为它代表无限远的环境，但如果需要，可以类似处理
    // if (sceneData_->skybox) { ... }

    return pickedObject;
}