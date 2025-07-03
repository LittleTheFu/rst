#ifndef _ROTATE_CAMERA_LEFT_COMMAND_H_
#define _ROTATE_CAMERA_LEFT_COMMAND_H_

#include "camera.h"
#include "command.h"

class RotateCameraLeftCommand : public ICommand {
public:
    RotateCameraLeftCommand(std::shared_ptr<Camera> camera, float deltaTime);
    void Execute() override;

private:
    std::shared_ptr<Camera> camera_;
    float deltaTime_;
};
#endif
