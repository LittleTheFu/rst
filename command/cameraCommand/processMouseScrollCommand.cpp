#include "processMouseScrollCommand.h"

ProcessMouseScrollCommand::ProcessMouseScrollCommand(std::shared_ptr<Camera> camera, float scrollY)
    : camera_(camera), scrollY_(scrollY)
{
}

void ProcessMouseScrollCommand::Execute()
{
    if (camera_)
    {
        camera_->ProcessMouseScroll(scrollY_);
    }
}