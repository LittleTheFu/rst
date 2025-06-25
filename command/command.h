#ifndef COMMAND_H
#define COMMAND_H

// 抽象命令基类
class Command {
public:
    virtual ~Command() = default; // 虚析构函数，确保派生类正确析构
    virtual void Execute() = 0;   // 纯虚函数，所有具体命令必须实现
};

#endif // COMMAND_H