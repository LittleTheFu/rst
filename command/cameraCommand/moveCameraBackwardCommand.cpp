#include "moveCameraBackwardCommand.h"

MoveCameraBackwardCommand::MoveCameraBackwardCommand(Camera *camera, float dt)
    : CameraCommand(camera, dt)
{
}

void MoveCameraBackwardCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(BACKWARD, deltaTime_);
}
