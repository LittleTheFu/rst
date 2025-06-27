#ifndef _ROTATE_CAMERA_LEFT_COMMAND_H_
#define _ROTATE_CAMERA_LEFT_COMMAND_H_

#include "cameraCommand.h"

class RotateCameraLeftCommand : public CameraCommand {
public:
    RotateCameraLeftCommand(std::shared_ptr<Camera> camera);
    void Execute() override;
};
#endif
