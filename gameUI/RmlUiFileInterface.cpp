// RmlUiFileInterface.cpp
#include "RmlUiFileInterface.h"
#include <iostream>

RmlUiFileInterface::RmlUiFileInterface(const std::string& root_path) : rootPath_(root_path) {
    // 确保 root_path 以斜杠结尾
    if (!rootPath_.empty() && rootPath_.back() != '/' && rootPath_.back() != '\\') {
        rootPath_ += '/';
    }
}

RmlUiFileInterface::~RmlUiFileInterface() {
    // 文件流由 RmlUi 负责关闭，或者在 Close 方法中处理
}

Rml::Core::FileHandle RmlUiFileInterface::Open(const Rml::Core::String& path) {
    std::string full_path = rootPath_ + path.CString();
    std::fstream* file = new std::fstream(full_path, std::ios::in | std::ios::binary);

    if (file->is_open()) {
        return (Rml::Core::FileHandle)file;
    } else {
        std::cerr << "Error: Failed to open RmlUi file: " << full_path << std::endl;
        delete file; // 释放内存
        return nullptr;
    }
}

void RmlUiFileInterface::Close(Rml::Core::FileHandle handle) {
    std::fstream* file = (std::fstream*)handle;
    if (file) {
        file->close();
        delete file;
    }
}

size_t RmlUiFileInterface::Read(void* data, size_t size, Rml::Core::FileHandle handle) {
    std::fstream* file = (std::fstream*)handle;
    if (file && file->is_open()) {
        file->read((char*)data, size);
        return (size_t)file->gcount(); // 返回实际读取的字节数
    }
    return 0;
}

void RmlUiFileInterface::Seek(Rml::Core::FileHandle handle, long offset, Rml::Core::File::SeekOrigin origin) {
    std::fstream* file = (std::fstream*)handle;
    if (file && file->is_open()) {
        std::ios_base::seekdir seek_dir;
        switch (origin) {
            case Rml::Core::File::SeekOrigin::SEEK_SET: seek_dir = std::ios_base::beg; break;
            case Rml::Core::File::SeekOrigin::SEEK_CUR: seek_dir = std::ios_base::cur; break;
            case Rml::Core::File::SeekOrigin::SEEK_END: seek_dir = std::ios_base::end; break;
            default: return; // 无效的 origin
        }
        file->seekg(offset, seek_dir);
    }
}

size_t RmlUiFileInterface::Tell(Rml::Core::FileHandle handle) {
    std::fstream* file = (std::fstream*)handle;
    if (file && file->is_open()) {
        return (size_t)file->tellg();
    }
    return 0;
}

size_t RmlUiFileInterface::Length(Rml::Core::FileHandle handle) {
    std::fstream* file = (std::fstream*)handle;
    if (file && file->is_open()) {
        size_t current_pos = file->tellg();
        file->seekg(0, std::ios::end);
        size_t length = file->tellg();
        file->seekg(current_pos, std::ios::beg); // 恢复到原来的位置
        return length;
    }
    return 0;
}
