#include "moveCameraDownCommand.h"

MoveCameraDownCommand::MoveCameraDownCommand(std::shared_ptr<Camera> camera, float dt)
    : CameraCommand(camera, dt)
{
}

void MoveCameraDownCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(DOWN, deltaTime_);
}