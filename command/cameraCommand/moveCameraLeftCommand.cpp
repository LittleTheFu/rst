#include "moveCameraLeftCommand.h"

MoveCameraLeftCommand::MoveCameraLeftCommand(std::shared_ptr<Camera> camera, float deltaTime)
    : camera_(camera), deltaTime_(deltaTime)
{
}

void MoveCameraLeftCommand::Execute()
{
    if (camera_)
        camera_->ProcessKeyboard(LEFT, deltaTime_);
}
