#include "processMouseMovementCommand.h"

ProcessMouseMovementCommand::ProcessMouseMovementCommand(std::shared_ptr<Camera> camera, float deltaX, float deltaY)
    : camera_(camera), deltaX_(deltaX), deltaY_(deltaY)
{
}

void ProcessMouseMovementCommand::Execute()
{
    if (camera_)
    {
        camera_->ProcessMouseMovement(deltaX_, deltaY_);
    }
}