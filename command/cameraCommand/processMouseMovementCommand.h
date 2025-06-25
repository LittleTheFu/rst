#ifndef _PROCESS_MOUSE_MOVEMENT_COMMAND_H_
#define _PROCESS_MOUSE_MOVEMENT_COMMAND_H_

#include "command.h"
#include "camera.h"
#include "inputManager.h"

class ProcessMouseMovementCommand : public Command {
private:
    Camera* camera_;

public:
    ProcessMouseMovementCommand(Camera* camera);
    void Execute() override;
};
#endif