#include "rotateCameraRightCommand.h"

RotateCameraRightCommand::RotateCameraRightCommand(std::shared_ptr<Camera> camera)
    : CameraCommand(camera)
{
}

void RotateCameraRightCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(ROT_RIGHT, deltaTime_);
}