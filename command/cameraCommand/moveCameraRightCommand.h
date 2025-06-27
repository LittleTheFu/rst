#ifndef _MOVE_CAMERA_RIGHT_COMMAND_H_
#define _MOVE_CAMERA_RIGHT_COMMAND_H_

#include "cameraCommand.h"

class MoveCameraRightCommand : public CameraCommand {
public:
    MoveCameraRightCommand(std::shared_ptr<Camera> camera);
    void Execute() override;
};
#endif
