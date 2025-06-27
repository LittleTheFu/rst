#include "moveCameraUpCommand.h"

MoveCameraUpCommand::MoveCameraUpCommand(std::shared_ptr<Camera> camera)
    : CameraCommand(camera)
{
}

void MoveCameraUpCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(UP, deltaTime_);
}