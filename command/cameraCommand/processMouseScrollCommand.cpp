#include "processMouseScrollCommand.h"

ProcessMouseScrollCommand::ProcessMouseScrollCommand(Camera *camera)
    : camera_(camera)
{
}

void ProcessMouseScrollCommand::Execute()
{
    if (camera_)
    {
        InputManager &input = InputManager::GetInstance();
        if (input.GetMouseScrollY() != 0)
        { // 只有滚轮滚动时才调用
            camera_->ProcessMouseScroll(input.GetMouseScrollY());
        }
    }
}