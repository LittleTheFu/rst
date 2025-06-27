#ifndef _MOVE_CAMERA_FORWARD_COMMAND_H_
#define _MOVE_CAMERA_FORWARD_COMMAND_H_

#include "cameraCommand.h"

class MoveCameraForwardCommand : public CameraCommand
{
public:
    MoveCameraForwardCommand(std::shared_ptr<Camera> camera);
    void Execute() override;
};
#endif
