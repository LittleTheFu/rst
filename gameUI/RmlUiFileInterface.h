// RmlUiFileInterface.h
#ifndef RMLUI_FILE_INTERFACE_H
#define RMLUI_FILE_INTERFACE_H

#include <RmlUi/Core/FileInterface.h>
#include <fstream> // 用于文件操作
#include <string>  // 用于文件路径

/**
 * @brief RmlUiFileInterface 实现了 RmlUi 的 FileInterface，
 * 负责从文件系统加载 RML 和 RCSS 文档。
 */
class RmlUiFileInterface : public Rml::Core::FileInterface {
public:
    RmlUiFileInterface(const std::string& root_path);
    virtual ~RmlUiFileInterface();

    /**
     * @brief 打开一个文件。
     * @param path 文件的相对路径。
     * @return 指向文件句柄的指针，如果文件无法打开则返回 nullptr。
     */
    Rml::Core::FileHandle Open(const Rml::Core::String& path) override;

    /**
     * @brief 关闭一个文件。
     * @param handle 文件句柄。
     */
    void Close(Rml::Core::FileHandle handle) override;

    /**
     * @brief 从文件中读取数据。
     * @param data 要读取到的缓冲区。
     * @param size 要读取的字节数。
     * @param handle 文件句柄。
     * @return 实际读取的字节数。
     */
    size_t Read(void* data, size_t size, Rml::Core::FileHandle handle) override;

    /**
     * @brief 在文件中查找。
     * @param handle 文件句柄。
     * @param offset 偏移量。
     * @param origin 查找的起始点（Rml::Core::File::SeekOrigin）。
     */
    void Seek(Rml::Core::FileHandle handle, long offset, Rml::Core::File::SeekOrigin origin) override;

    /**
     * @brief 获取文件中的当前位置。
     * @param handle 文件句柄。
     * @return 当前文件位置。
     */
    size_t Tell(Rml::Core::FileHandle handle) override;

    /**
     * @brief 获取文件的大小。
     * @param handle 文件句柄。
     * @return 文件大小（字节）。
     */
    size_t Length(Rml::Core::FileHandle handle) override;

private:
    std::string rootPath_; // 文件系统的根路径，RmlUi 将在此路径下查找文件
};

#endif // RMLUI_FILE_INTERFACE_H