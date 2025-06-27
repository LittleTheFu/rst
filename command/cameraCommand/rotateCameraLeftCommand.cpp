#include "rotateCameraLeftCommand.h"

RotateCameraLeftCommand::RotateCameraLeftCommand(std::shared_ptr<Camera> camera, float dt)
    : CameraCommand(camera, dt)
{
}

void RotateCameraLeftCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(ROT_LEFT, deltaTime_);
}