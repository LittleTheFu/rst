#include "pickObjectCommand.h"
#include "sceneObject.h" // 确保包含 ISceneObject，因为要使用其 getName()

PickObjectCommand::PickObjectCommand(ObjectPicker *picker)
    : picker_(picker), mouseX_(0), mouseY_(0), pickedObject_(nullptr) // !!! 关键改动 !!! 初始化 pickedObject_
{
}

void PickObjectCommand::Execute()
{
    if (picker_)
    {
        // !!! 关键改动 !!!
        // picker_->pick 现在应该返回 ISceneObject*
        pickedObject_ = picker_->pick(mouseX_, mouseY_); 
        if (pickedObject_) // 现在检查 pickedObject_
        {
            // !!! 关键改动 !!!
            // 使用 pickedObject_ 的 getName() 方法
            std::cout << "Command executed: Picked object: " << pickedObject_->getName() << std::endl;
        }
        else
        {
            std::cout << "Command executed: No object picked." << std::endl;
        }
    }
}

void PickObjectCommand::setMousePosition(int mouseX, int mouseY)
{
    mouseX_ = mouseX;
    mouseY_ = mouseY;
}