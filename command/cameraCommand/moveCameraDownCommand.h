#ifndef _MOVE_CAMERA_DOWN_COMMAND_H_
#define _MOVE_CAMERA_DOWN_COMMAND_H_

#include "command.h"
#include "camera.h"

class MoveCameraDownCommand : public ICommand
{
public:
    MoveCameraDownCommand(std::shared_ptr<Camera> camera, float deltaTime);
    void Execute() override;

private:
    std::shared_ptr<Camera> camera_;
    float deltaTime_;
};
#endif