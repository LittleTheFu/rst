#include "rotateCameraRightCommand.h"

RotateCameraRightCommand::RotateCameraRightCommand(std::shared_ptr<Camera> camera, float dt)
    : CameraCommand(camera, dt)
{
}

void RotateCameraRightCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(ROT_RIGHT, deltaTime_);
}