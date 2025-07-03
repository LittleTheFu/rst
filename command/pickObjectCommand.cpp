#include "pickObjectCommand.h"
#include "sceneObject.h" // 确保包含 ISceneObject，因为要使用其 getName()

PickObjectCommand::PickObjectCommand(ObjectPicker *picker, int mouseX, int mouseY)
    : picker_(picker), mouseX_(mouseX), mouseY_(mouseY), pickedObject_(nullptr) // !!! 关键改动 !!! 初始化 pickedObject_
{
}

void PickObjectCommand::Execute()
{
    if (picker_)
    {
        pickedObject_ = picker_->pick(mouseX_, mouseY_); 
        if (pickedObject_)
        {
            std::cout << "Command executed: Picked object: " << pickedObject_->getName() << std::endl;
        }
        else
        {
            std::cout << "Command executed: No object picked." << std::endl;
        }
    }
}