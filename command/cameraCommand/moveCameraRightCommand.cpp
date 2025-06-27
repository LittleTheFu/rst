#include "moveCameraRightCommand.h"

MoveCameraRightCommand::MoveCameraRightCommand(std::shared_ptr<Camera> camera)
    : CameraCommand(camera)
{
}

void MoveCameraRightCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(RIGHT, deltaTime_);
}
