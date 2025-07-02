#ifndef _PROCESS_MOUSE_MOVEMENT_COMMAND_H_
#define _PROCESS_MOUSE_MOVEMENT_COMMAND_H_

#include "command.h"
#include "camera.h"
#include "inputManager.h"

class ProcessMouseMovementCommand : public ICommand {
private:
    std::shared_ptr<Camera> camera_;

public:
    ProcessMouseMovementCommand(std::shared_ptr<Camera> camera);
    void Execute() override;
};
#endif