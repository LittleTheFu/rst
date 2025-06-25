#include "processMouseMovementCommand.h"

ProcessMouseMovementCommand::ProcessMouseMovementCommand(Camera *camera)
    : camera_(camera)
{
}

void ProcessMouseMovementCommand::Execute()
{
    if (camera_)
    {
        InputManager &input = InputManager::GetInstance();
        camera_->ProcessMouseMovement(input.GetMouseDeltaX(), input.GetMouseDeltaY());
    }
}