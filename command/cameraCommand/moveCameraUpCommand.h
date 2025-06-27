#ifndef _MOVE_CAMERA_UP_COMMAND_H_
#define _MOVE_CAMERA_UP_COMMAND_H_

#include "cameraCommand.h"

class MoveCameraUpCommand : public CameraCommand {
public:
    MoveCameraUpCommand(std::shared_ptr<Camera> camera);
    void Execute() override;
};
#endif
