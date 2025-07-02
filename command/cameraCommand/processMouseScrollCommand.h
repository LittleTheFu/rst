#ifndef _PROCESS_MOUSE_SCROLL_COMMAND_H_
#define _PROCESS_MOUSE_SCROLL_COMMAND_H_

#include "command.h"
#include "camera.h"
#include "inputManager.h"

class ProcessMouseScrollCommand : public ICommand
{
private:
    std::shared_ptr<Camera> camera_;

public:
    ProcessMouseScrollCommand(std::shared_ptr<Camera> camera);
    void Execute() override;
};
#endif