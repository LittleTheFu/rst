#include "moveCameraDownCommand.h"

MoveCameraDownCommand::MoveCameraDownCommand(std::shared_ptr<Camera> camera)
    : CameraCommand(camera)
{
}

void MoveCameraDownCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(DOWN, deltaTime_);
}