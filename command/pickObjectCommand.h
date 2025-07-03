#ifndef PICK_OBJECT_COMMAND_H
#define PICK_OBJECT_COMMAND_H

#include "Command.h"       // 假设你有一个 Command 接口或基类
#include "ObjectPicker.h"  // 你的 ObjectPicker 类 (它应该返回 ISceneObject*)
#include "sceneObject.h"  // 引入 ISceneObject 接口
#include <iostream>        // For demo output

class PickObjectCommand : public ICommand {
public:
    PickObjectCommand(ObjectPicker* picker, int mouseX, int mouseY);
    void Execute() override;
    
    // ISceneObject* getPickedObject() const { return pickedObject_; } 

private:
    ObjectPicker* picker_;
    int mouseX_;
    int mouseY_;
    
    // !!! 关键改动 !!!
    // 存储拾取结果现在是 ISceneObject*
    ISceneObject* pickedObject_; 
};

#endif // PICK_OBJECT_COMMAND_H