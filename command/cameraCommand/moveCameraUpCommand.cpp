#include "moveCameraUpCommand.h"

MoveCameraUpCommand::MoveCameraUpCommand(std::shared_ptr<Camera> camera, float deltaTime)
    : camera_(camera), deltaTime_(deltaTime)
{
}

void MoveCameraUpCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(UP, deltaTime_);
}