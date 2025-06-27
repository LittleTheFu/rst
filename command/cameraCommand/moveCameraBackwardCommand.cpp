#include "moveCameraBackwardCommand.h"

MoveCameraBackwardCommand::MoveCameraBackwardCommand(std::shared_ptr<Camera> camera)
    : CameraCommand(camera)
{
}

void MoveCameraBackwardCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(BACKWARD, deltaTime_);
}
