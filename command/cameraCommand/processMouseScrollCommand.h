#ifndef _PROCESS_MOUSE_SCROLL_COMMAND_H_
#define _PROCESS_MOUSE_SCROLL_COMMAND_H_

#include "command.h"
#include "camera.h"
#include "inputManager.h"

class ProcessMouseScrollCommand : public Command
{
private:
    Camera *camera_;

public:
    ProcessMouseScrollCommand(Camera *camera);
    void Execute() override;
};
#endif