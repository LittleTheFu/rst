#include "moveCameraUpCommand.h"

MoveCameraUpCommand::MoveCameraUpCommand(Camera *camera, float dt)
    : CameraCommand(camera, dt)
{
}

void MoveCameraUpCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(UP, deltaTime_);
}