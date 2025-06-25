#ifndef _MOVE_CAMERA_LEFT_COMMAND_H_
#define _MOVE_CAMERA_LEFT_COMMAND_H_

#include "cameraCommand.h"

class MoveCameraLeftCommand : public CameraCommand
{
public:
    MoveCameraLeftCommand(Camera *camera, float dt);
    void Execute() override;
};
#endif
