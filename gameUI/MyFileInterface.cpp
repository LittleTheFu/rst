#include "MyFileInterface.h"
#include <RmlUi/Core/Log.h>       // For Rml::Log::Message
#include <filesystem>     // C++17 for path manipulation
#include <errno.h>        // For errno and strerror
#include <string.h>       // For strerror


MyFileInterface::MyFileInterface() {
    // Constructor
}

Rml::FileHandle MyFileInterface::Open(const Rml::String& path)
{
    // Resource path handling: RmlUi requested path will be prefixed with 'resource/'
    // Assuming the executable's current working directory, or that 'resource' is in the same directory as the executable.

    std::string fullPath;
    try {
        std::filesystem::path base_resource_dir = "resource";
        std::filesystem::path requested_path(path.c_str());
        std::filesystem::path final_path = base_resource_dir / requested_path;
        fullPath = final_path.string();
    } catch (const std::filesystem::filesystem_error& e) {
        // Log error using RmlUi's logging system
        Rml::Log::Message(Rml::Log::LT_ERROR, "Filesystem error while constructing path '%s': %s", path.c_str(), e.what());
        // 遵循编译器指示：使用 reinterpret_cast
        return reinterpret_cast<Rml::FileHandle>(nullptr);
    }

    FILE* file = fopen(fullPath.c_str(), "rb"); // "rb" for binary read mode

    if (!file) {
        // Log failure using RmlUi's logging system
        // strerror(errno) provides specific system error messages (e.g., "No such file or directory")
        Rml::Log::Message(Rml::Log::LT_WARNING, "Failed to open file: '%s'. Full path attempted: '%s'. Error: %s",
            path.c_str(), fullPath.c_str(), strerror(errno));
        // 遵循编译器指示：使用 reinterpret_cast
        return reinterpret_cast<Rml::FileHandle>(nullptr);
    }
    return reinterpret_cast<Rml::FileHandle>(file);
}

void MyFileInterface::Close(Rml::FileHandle file)
{
    if (file) {
        fclose(reinterpret_cast<FILE*>(file));
    }
}

size_t MyFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file)
{
    if (file) {
        return fread(buffer, 1, size, reinterpret_cast<FILE*>(file));
    }
    return 0;
}

bool MyFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
{
    if (file) {
        return fseek(reinterpret_cast<FILE*>(file), offset, origin) == 0;
    }
    return false;
}

size_t MyFileInterface::Tell(Rml::FileHandle file)
{
    if (file) {
        long pos = ftell(reinterpret_cast<FILE*>(file));
        return (pos == -1L) ? 0 : static_cast<size_t>(pos); // -1L indicates error
    }
    return 0;
}