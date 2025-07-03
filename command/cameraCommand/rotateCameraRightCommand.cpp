#include "rotateCameraRightCommand.h"

RotateCameraRightCommand::RotateCameraRightCommand(std::shared_ptr<Camera> camera, float deltaTime)
    : camera_(camera), deltaTime_(deltaTime)
{
}

void RotateCameraRightCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(ROT_RIGHT, deltaTime_);
}