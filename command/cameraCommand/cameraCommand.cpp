#include "cameraCommand.h"

CameraCommand::CameraCommand(std::shared_ptr<Camera> camera)
    : camera_(camera)
{
}

void CameraCommand::setDeltaTime(float deltaTime)
{
    deltaTime_ = deltaTime;
}
