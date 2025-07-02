#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#include "command.h" // 包含抽象命令接口
#include <queue>      // 用于存储命令
#include <memory>     // 用于智能指针 std::unique_ptr

// 命令队列类，负责存储和执行命令
class CommandQueue {
public:
    // 将一个命令添加到队列
    // 使用 std::unique_ptr 确保命令的所有权被转移到队列，并在执行后自动销毁
    void AddCommand(std::unique_ptr<ICommand> command) {
        commands_.push(std::move(command));
    }

    // 处理并执行队列中的所有命令
    // 每帧调用一次，确保所有命令在逻辑更新前执行
    void ProcessCommands() {
        while (!commands_.empty()) {
            commands_.front()->Execute(); // 执行队列头部的命令
            commands_.pop();             // 从队列中移除已执行的命令
        }
    }

    // 清空队列（通常在每帧开始时调用，如果不需要执行前一帧剩余的命令）
    void Clear() {
        std::queue<std::unique_ptr<ICommand>> emptyQueue;
        std::swap(commands_, emptyQueue); // 交换到空队列，高效清空
    }

private:
    std::queue<std::unique_ptr<ICommand>> commands_; // 存储命令的队列
};

#endif // COMMAND_QUEUE_H