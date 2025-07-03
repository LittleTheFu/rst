#ifndef _ROTATE_CAMERA_RIGHT_COMMAND_H_
#define _ROTATE_CAMERA_RIGHT_COMMAND_H_

#include "camera.h"
#include "command.h"

class RotateCameraRightCommand : public ICommand {
public:
    RotateCameraRightCommand(std::shared_ptr<Camera> camera, float deltaTime);
    void Execute() override;

private:
    std::shared_ptr<Camera> camera_;
    float deltaTime_;
};
#endif
