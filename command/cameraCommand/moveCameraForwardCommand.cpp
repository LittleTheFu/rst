#include "moveCameraForwardCommand.h"

MoveCameraForwardCommand::MoveCameraForwardCommand(std::shared_ptr<Camera> camera)
    : CameraCommand(camera)
{
}

void MoveCameraForwardCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(FORWARD, deltaTime_);
}