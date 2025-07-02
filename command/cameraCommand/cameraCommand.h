#ifndef _CAMERA_COMMAND_H_
#define _CAMERA_COMMAND_H_

#include "command.h"
#include "camera.h"

class CameraCommand : public ICommand 
{
public:
    void setDeltaTime(float deltaTime);

protected:
    std::shared_ptr<Camera> camera_;
    float deltaTime_; // 命令可能需要 deltaTime 来进行时间相关的操作

public:
    CameraCommand(std::shared_ptr<Camera> camera);
    // Execute 仍由派生类实现
};

#endif