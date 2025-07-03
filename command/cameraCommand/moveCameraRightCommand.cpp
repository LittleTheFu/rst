#include "moveCameraRightCommand.h"

MoveCameraRightCommand::MoveCameraRightCommand(std::shared_ptr<Camera> camera, float deltaTime)
    : camera_(camera), deltaTime_(deltaTime)
{
}

void MoveCameraRightCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(RIGHT, deltaTime_);
}
