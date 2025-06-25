#include "cameraCommand.h"

CameraCommand::CameraCommand(Camera *camera, float dt) : camera_(camera), deltaTime_(dt)
{
}

void CameraCommand::setDeltaTime(float deltaTime)
{
    deltaTime_ = deltaTime;
}
