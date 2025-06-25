#ifndef _TOGGLE_DEBUG_MODE_COMMAND_H_
#define _TOGGLE_DEBUG_MODE_COMMAND_H_

#include "command.h"
#include "scene.h"

class ToggleDebugModeCommand : public Command
{
    // Scene* scene_; // 假设有一个场景指针
public:
    // ToggleDebugModeCommand(Scene* scene) : scene_(scene) {}
    void Execute() override;
};
#endif