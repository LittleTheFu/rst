#include "processMouseMovementCommand.h"

ProcessMouseMovementCommand::ProcessMouseMovementCommand(std::shared_ptr<Camera> camera)
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