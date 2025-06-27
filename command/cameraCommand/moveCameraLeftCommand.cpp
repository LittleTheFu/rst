#include "moveCameraLeftCommand.h"

MoveCameraLeftCommand::MoveCameraLeftCommand(std::shared_ptr<Camera> camera, float dt)
    : CameraCommand(camera, dt)
{
}

void MoveCameraLeftCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(LEFT, deltaTime_);
}
