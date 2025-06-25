#ifndef _MOVE_CAMERA_DOWN_COMMAND_H_
#define _MOVE_CAMERA_DOWN_COMMAND_H_

#include "cameraCommand.h"

class MoveCameraDownCommand : public CameraCommand
{
public:
    MoveCameraDownCommand(Camera *camera, float dt);
    void Execute() override;
};
#endif