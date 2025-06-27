#include "toggleDebugModeCommand.h"

ToggleDebugModeCommand::ToggleDebugModeCommand(std::shared_ptr<Scene> scene)
    : scene_(scene)
{
}

void ToggleDebugModeCommand::Execute()
{
    std::cout << "Debug Mode Toggled!" << std::endl;
    // if (scene_) scene_->toggleDebugMode(); // 调用场景的调试模式切换方法
}