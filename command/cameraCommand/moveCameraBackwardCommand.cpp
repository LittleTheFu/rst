#include "moveCameraBackwardCommand.h"

MoveCameraBackwardCommand::MoveCameraBackwardCommand(std::shared_ptr<Camera> camera, float dt)
    : CameraCommand(camera, dt)
{
}

void MoveCameraBackwardCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(BACKWARD, deltaTime_);
}
