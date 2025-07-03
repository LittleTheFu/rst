#ifndef _PROCESS_MOUSE_MOVEMENT_COMMAND_H_
#define _PROCESS_MOUSE_MOVEMENT_COMMAND_H_

#include "command.h"
#include "camera.h"
#include "inputManager.h"

class ProcessMouseMovementCommand : public ICommand
{
public:
    ProcessMouseMovementCommand(std::shared_ptr<Camera> camera, float deltaX, float deltaY);
    void Execute() override;

private:
    std::shared_ptr<Camera> camera_;
    float deltaX_;
    float deltaY_;
};
#endif