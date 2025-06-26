#include "pickObjectCommand.h"

PickObjectCommand::PickObjectCommand(ObjectPicker *picker)
    : picker_(picker), mouseX_(0), mouseY_(0), pickedMesh_(nullptr)
{
}

void PickObjectCommand::Execute()
{
    if (picker_)
    {
        pickedMesh_ = picker_->pick(mouseX_, mouseY_);
        if (pickedMesh_)
        {
            // 在这里处理拾取结果，例如：
            // 1. 设置场景中当前选中的物体
            // 2. 触发一个事件通知其他系统物体被选中
            // 3. (调试用) 打印信息
            std::cout << "Command executed: Picked mesh at address: " << pickedMesh_ << std::endl;
            // 如果 Mesh 有 getName() 方法，可以打印：
            // std::cout << "Command executed: Picked mesh: " << pickedMesh_->getName() << std::endl;

            // 示例：可以把 pickedMesh_ 传递给一个 Scene 管理器或者 SelectionManager
            // SceneManager::getInstance()->setSelectedMesh(pickedMesh_);
        }
        else
        {
            std::cout << "Command executed: No mesh picked." << std::endl;
        }
    }
}

void PickObjectCommand::setMousePosition(int mouseX, int mouseY)
{
    mouseX_ = mouseX;
    mouseY_ = mouseY;
}
