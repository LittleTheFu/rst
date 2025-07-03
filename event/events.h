// Events.h 或独立的事件文件
#ifndef GAME_EVENTS_H
#define GAME_EVENTS_H

#include "sceneObject.h" // 确保包含 ISceneObject

namespace GameEvents {

struct ObjectPickedEvent {
    ISceneObject* pickedObject; // 拾取到的对象，如果未拾取到可以是 nullptr
    int mouseX;                 // 鼠标X坐标（可选，但通常有用）
    int mouseY;                 // 鼠标Y坐标（可选）

    // 如果需要，可以添加更多信息，如：
    // float hitDistance;
    // Eigen::Vector3f hitNormal;
};

// ... 其他事件类型，如 PlayerDiedEvent, LevelLoadedEvent 等

} // namespace GameEvents

#endif // GAME_EVENTS_H