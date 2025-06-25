#ifndef _MOVE_CAMERA_FORWARD_COMMAND_H_
#define _MOVE_CAMERA_FORWARD_COMMAND_H_

#include "cameraCommand.h"

class MoveCameraForwardCommand : public CameraCommand
{
public:
    MoveCameraForwardCommand(Camera *camera, float dt);
    void Execute() override;
};
#endif
