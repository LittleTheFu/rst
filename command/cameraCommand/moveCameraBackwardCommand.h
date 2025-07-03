#ifndef _MOVE_CAMERA_BACKWARD_COMMAND_H_
#define _MOVE_CAMERA_BACKWARD_COMMAND_H_

#include "command.h"
#include "camera.h"

class MoveCameraBackwardCommand : public ICommand {
public:
    MoveCameraBackwardCommand(std::shared_ptr<Camera> camera, float deltaTime);
    void Execute() override;

private:
    std::shared_ptr<Camera> camera_;
    float deltaTime_;
};
#endif
