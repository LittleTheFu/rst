#ifndef _PROCESS_MOUSE_SCROLL_COMMAND_H_
#define _PROCESS_MOUSE_SCROLL_COMMAND_H_

#include "command.h"
#include "camera.h"

class ProcessMouseScrollCommand : public ICommand
{
public:
    ProcessMouseScrollCommand(std::shared_ptr<Camera> camera, float scrollY);
    void Execute() override;

private:
    std::shared_ptr<Camera> camera_;
    float scrollY_;
};
#endif