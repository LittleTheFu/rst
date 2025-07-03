#ifndef _MOVE_CAMERA_UP_COMMAND_H_
#define _MOVE_CAMERA_UP_COMMAND_H_

#include "command.h"
#include "camera.h"

class MoveCameraUpCommand : public ICommand {
public:
    MoveCameraUpCommand(std::shared_ptr<Camera> camera, float deltaTime);
    void Execute() override;

private:
    std::shared_ptr<Camera> camera_;
    float deltaTime_;
};
#endif
