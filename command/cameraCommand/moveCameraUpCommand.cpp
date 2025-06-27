#include "moveCameraUpCommand.h"

MoveCameraUpCommand::MoveCameraUpCommand(std::shared_ptr<Camera> camera, float dt)
    : CameraCommand(camera, dt)
{
}

void MoveCameraUpCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(UP, deltaTime_);
}