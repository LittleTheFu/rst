#ifndef _MOVE_CAMERA_RIGHT_COMMAND_H_
#define _MOVE_CAMERA_RIGHT_COMMAND_H_

#include "command.h"
#include "camera.h"

class MoveCameraRightCommand : public ICommand {
public:
    MoveCameraRightCommand(std::shared_ptr<Camera> camera, float deltaTime);
    void Execute() override;

private:
    std::shared_ptr<Camera> camera_;
    float deltaTime_;
};
#endif
