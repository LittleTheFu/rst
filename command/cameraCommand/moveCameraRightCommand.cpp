#include "moveCameraRightCommand.h"

MoveCameraRightCommand::MoveCameraRightCommand(Camera *camera, float dt)
    : CameraCommand(camera, dt)
{
}

void MoveCameraRightCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(RIGHT, deltaTime_);
}
