#include "moveCameraForwardCommand.h"

MoveCameraForwardCommand::MoveCameraForwardCommand(std::shared_ptr<Camera> camera, float dt)
    : CameraCommand(camera, dt)
{
}

void MoveCameraForwardCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(FORWARD, deltaTime_);
}