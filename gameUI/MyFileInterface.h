#pragma once

#include <RmlUi/Core/FileInterface.h>
#include <cstdio> // For FILE* and standard file I/O functions

class MyFileInterface : public Rml::FileInterface
{
public:
    MyFileInterface();
    ~MyFileInterface() = default;

    // Rml::FileInterface 虚函数实现
    Rml::FileHandle Open(const Rml::String& path) override;
    void Close(Rml::FileHandle file) override;
    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
    bool Seek(Rml::FileHandle file, long offset, int origin) override;
    size_t Tell(Rml::FileHandle file) override;

    // Length 和 LoadFile 在基类中有默认实现，它们会调用 Open/Close/Read/Seek/Tell。
    // 因此，通常不需要在这里 override。
};