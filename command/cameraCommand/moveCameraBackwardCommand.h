#ifndef _MOVE_CAMERA_BACKWARD_COMMAND_H_
#define _MOVE_CAMERA_BACKWARD_COMMAND_H_

#include "cameraCommand.h"

class MoveCameraBackwardCommand : public CameraCommand {
public:
    MoveCameraBackwardCommand(std::shared_ptr<Camera> camera, float dt);
    void Execute() override;
};
#endif
