#include "rotateCameraLeftCommand.h"

RotateCameraLeftCommand::RotateCameraLeftCommand(std::shared_ptr<Camera> camera)
    : CameraCommand(camera)
{
}

void RotateCameraLeftCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(ROT_LEFT, deltaTime_);
}