#ifndef _MOVE_CAMERA_RIGHT_COMMAND_H_
#define _MOVE_CAMERA_RIGHT_COMMAND_H_

#include "cameraCommand.h"

class MoveCameraRightCommand : public CameraCommand {
public:
    MoveCameraRightCommand(Camera *camera, float dt);
    void Execute() override;
};
#endif
