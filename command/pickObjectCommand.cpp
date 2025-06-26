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
            std::cout << "Command executed: Picked mesh: " << pickedMesh_->getName() << std::endl;
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
