#include "moveCameraLeftCommand.h"

MoveCameraLeftCommand::MoveCameraLeftCommand(std::shared_ptr<Camera> camera)
    : CameraCommand(camera)
{
}

void MoveCameraLeftCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(LEFT, deltaTime_);
}
