#include "toggleDebugModeCommand.h"

void ToggleDebugModeCommand::Execute()
{
    std::cout << "Debug Mode Toggled!" << std::endl;
    // if (scene_) scene_->toggleDebugMode(); // 调用场景的调试模式切换方法
}