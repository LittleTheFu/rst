// RmlUiFileInterface.h (最终修正版)
#ifndef RMLUI_FILE_INTERFACE_H
#define RMLUI_FILE_INTERFACE_H

// 注意：这里是 Rml::FileInterface，而不是 Rml::Core::FileInterface
#include <RmlUi/Core/FileInterface.h> 
#include <fstream>
#include <string>
#include <cstdio> // 用于 SEEK_SET, SEEK_CUR, SEEK_END

// 确保 Rml::String 和 Rml::FileHandle 在这里是可见的
// 如果它们不在 RmlUi/Core/FileInterface.h 中定义，你可能需要包含 RmlUi/Core.h
// 或者在 RmlUiFileInterface.cpp 中包含 RmlUi/Core.h
// 为了避免重复包含，这里假设 FileInterface.h 已经包含了必要的 Rml:: 类型

class RmlUiFileInterface : public Rml::FileInterface { // <--- 修正基类为 Rml::FileInterface
public:
    RmlUiFileInterface(const std::string& root_path);
    virtual ~RmlUiFileInterface();

    // 纯虚函数签名，与 Rml::FileInterface 匹配
    Rml::FileHandle Open(const Rml::String& path) override;
    void Close(Rml::FileHandle file) override;
    size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
    
    // <--- 修正 Seek 函数签名：返回 bool，origin 类型为 int
    bool Seek(Rml::FileHandle file, long offset, int origin) override; 
    
    size_t Tell(Rml::FileHandle file) override;
    size_t Length(Rml::FileHandle file) override;

private:
    std::string rootPath_;
};

#endif