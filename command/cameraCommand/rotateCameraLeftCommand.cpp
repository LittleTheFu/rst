#include "rotateCameraLeftCommand.h"

RotateCameraLeftCommand::RotateCameraLeftCommand(std::shared_ptr<Camera> camera, float deltaTime)
    : camera_(camera), deltaTime_(deltaTime)
{
}

void RotateCameraLeftCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(ROT_LEFT, deltaTime_);
}