#include "pickObjectCommand.h"
#include "sceneObject.h" // 确保包含 ISceneObject，因为要使用其 getName()
#include <events.h>
#include <EventBus.h>

PickObjectCommand::PickObjectCommand(ObjectPicker *picker, int mouseX, int mouseY)
    : picker_(picker), mouseX_(mouseX), mouseY_(mouseY)
{
}

void PickObjectCommand::Execute()
{
    if (picker_)
    {
        ISceneObject* pickedObject = picker_->pick(mouseX_, mouseY_);
        EventBus::GetInstance().Publish(GameEvents::ObjectPickedEvent{pickedObject, mouseX_, mouseY_});

        if (pickedObject)
        {
            std::cout << "Command executed: Picked object: " << pickedObject->getName() << std::endl;
        }
        else
        {
            std::cout << "Command executed: No object picked." << std::endl;
        }
    }
}