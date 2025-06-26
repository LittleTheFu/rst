// PickObjectCommand.h
#ifndef PICK_OBJECT_COMMAND_H
#define PICK_OBJECT_COMMAND_H

#include "Command.h"       // 假设你有一个 Command 接口或基类
#include "ObjectPicker.h"  // 你的 ObjectPicker 类
#include "Mesh.h"          // Mesh 类
#include <iostream>        // For demo output

class PickObjectCommand : public Command {
public:
    PickObjectCommand(ObjectPicker* picker);
    void Execute() override;

    //should I add this interface below?I'll come back later....
    void setMousePosition(int mouseX, int mouseY);
    Mesh* getPickedMesh() const { return pickedMesh_; } // 方便外部获取结果

private:
    ObjectPicker* picker_;
    int mouseX_;
    int mouseY_;
    Mesh* pickedMesh_; // 存储拾取结果
};

#endif // PICK_OBJECT_COMMAND_H