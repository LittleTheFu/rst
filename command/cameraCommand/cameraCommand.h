#ifndef _CAMERA_COMMAND_H_
#define _CAMERA_COMMAND_H_

#include "command.h"
#include "camera.h"

class CameraCommand : public Command {
protected:
    Camera* camera_;
    float deltaTime_; // 命令可能需要 deltaTime 来进行时间相关的操作

public:
    CameraCommand(Camera* camera, float dt);
    // Execute 仍由派生类实现
};

#endif