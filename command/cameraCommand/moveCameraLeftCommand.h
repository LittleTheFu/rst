#ifndef _MOVE_CAMERA_LEFT_COMMAND_H_
#define _MOVE_CAMERA_LEFT_COMMAND_H_

#include "cameraCommand.h"

class MoveCameraLeftCommand : public CameraCommand
{
public:
    MoveCameraLeftCommand(std::shared_ptr<Camera> camera, float dt);
    void Execute() override;
};
#endif
