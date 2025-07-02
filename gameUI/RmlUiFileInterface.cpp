#include "RmlUiFileInterface.h"
#include <iostream>
#include <cstdio> // 用于 SEEK_SET, SEEK_CUR, SEEK_END

RmlUiFileInterface::RmlUiFileInterface(const std::string& root_path) : rootPath_(root_path) {
    if (!rootPath_.empty() && rootPath_.back() != '/' && rootPath_.back() != '\\') {
        rootPath_ += '/';
    }
}

RmlUiFileInterface::~RmlUiFileInterface() {
}

Rml::FileHandle RmlUiFileInterface::Open(const Rml::String& path) {
    std::string full_path = rootPath_ + path;
    std::fstream* file = new std::fstream(full_path, std::ios::in | std::ios::binary);

    if (file->is_open()) {
        return (Rml::FileHandle)file;
    } else {
        std::cerr << "Error: Failed to open RmlUi file: " << full_path << std::endl;
        delete file;
        return (Rml::FileHandle)0; // 使用 0 作为无效句柄
    }
}

void RmlUiFileInterface::Close(Rml::FileHandle handle) {
    std::fstream* file = (std::fstream*)handle;
    if (file) {
        file->close();
        delete file;
    }
}

size_t RmlUiFileInterface::Read(void* data, size_t size, Rml::FileHandle handle) {
    std::fstream* file = (std::fstream*)handle;
    if (file && file->is_open()) {
        file->read((char*)data, size);
        return (size_t)file->gcount();
    }
    return 0;
}

// <--- 修正 Seek 函数实现：返回 bool，origin 类型为 int
bool RmlUiFileInterface::Seek(Rml::FileHandle handle, long offset, int origin) {
    std::fstream* file = (std::fstream*)handle;
    if (file && file->is_open()) {
        std::ios_base::seekdir seek_dir;
        switch (origin) {
            case SEEK_SET: seek_dir = std::ios_base::beg; break; // <--- 使用 C 标准库的宏
            case SEEK_CUR: seek_dir = std::ios_base::cur; break; // <--- 使用 C 标准库的宏
            case SEEK_END: seek_dir = std::ios_base::end; break; // <--- 使用 C 标准库的宏
            default: return false; // 无效的 origin，返回 false
        }
        file->seekg(offset, seek_dir);
        return !file->fail(); // 如果 seekg 失败，则返回 false
    }
    return false; // 文件句柄无效或文件未打开，返回 false
}

size_t RmlUiFileInterface::Tell(Rml::FileHandle handle) {
    std::fstream* file = (std::fstream*)handle;
    if (file && file->is_open()) {
        return (size_t)file->tellg();
    }
    return 0;
}

size_t RmlUiFileInterface::Length(Rml::FileHandle handle) {
    std::fstream* file = (std::fstream*)handle;
    if (file && file->is_open()) {
        size_t current_pos = file->tellg();
        file->seekg(0, std::ios::end);
        size_t length = file->tellg();
        file->seekg(current_pos, std::ios::beg);
        return length;
    }
    return 0;
}
