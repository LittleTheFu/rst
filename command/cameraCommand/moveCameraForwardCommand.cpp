#include "moveCameraForwardCommand.h"

MoveCameraForwardCommand::MoveCameraForwardCommand(std::shared_ptr<Camera> camera, float deltaTime)
    : camera_(camera), deltaTime_(deltaTime)
{
}

void MoveCameraForwardCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(FORWARD, deltaTime_);
}