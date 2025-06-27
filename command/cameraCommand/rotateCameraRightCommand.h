#ifndef _ROTATE_CAMERA_RIGHT_COMMAND_H_
#define _ROTATE_CAMERA_RIGHT_COMMAND_H_

#include "cameraCommand.h"

class RotateCameraRightCommand : public CameraCommand {
public:
    RotateCameraRightCommand(std::shared_ptr<Camera> camera);
    void Execute() override;
};
#endif
