#ifndef PICK_OBJECT_COMMAND_H
#define PICK_OBJECT_COMMAND_H

#include "Command.h"       // 假设你有一个 Command 接口或基类
#include "ObjectPicker.h"  // 你的 ObjectPicker 类 (它应该返回 ISceneObject*)
#include "sceneObject.h"  // 引入 ISceneObject 接口
#include <iostream>        // For demo output

class PickObjectCommand : public Command {
public:
    PickObjectCommand(ObjectPicker* picker);
    void Execute() override;

    void setMousePosition(int mouseX, int mouseY);
    
    // !!! 关键改动 !!!
    // 返回 ISceneObject* 而不是 Mesh*
    ISceneObject* getPickedObject() const { return pickedObject_; } 

private:
    ObjectPicker* picker_;
    int mouseX_;
    int mouseY_;
    
    // !!! 关键改动 !!!
    // 存储拾取结果现在是 ISceneObject*
    ISceneObject* pickedObject_; 
};

#endif // PICK_OBJECT_COMMAND_H