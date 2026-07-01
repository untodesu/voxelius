#include "core/pch.hh"

#include "core/utils/physfs.hh"

bool utils::read_file(std::string_view path, std::string& buffer)
{
    auto path_unfucked = std::string(path);
    auto file = PHYSFS_openRead(path_unfucked.c_str());

    if(file == nullptr) {
        LOG_WARNING("{}: {}", path_unfucked, physfs_error());
        return false;
    }

    buffer.resize(1 + PHYSFS_fileLength(file), static_cast<char>(0x00));

    PHYSFS_readBytes(file, buffer.data(), buffer.size());
    PHYSFS_close(file);

    return true;
}

bool utils::read_file(std::string_view path, std::vector<std::byte>& buffer)
{
    auto path_unfucked = std::string(path);
    auto file = PHYSFS_openRead(path_unfucked.c_str());

    if(file == nullptr) {
        LOG_WARNING("{}: {}", path_unfucked, physfs_error());
        return false;
    }

    buffer.resize(1 + PHYSFS_fileLength(file), static_cast<std::byte>(0x00));

    PHYSFS_readBytes(file, buffer.data(), buffer.size());
    PHYSFS_close(file);

    return true;
}

bool utils::read_file(std::string_view path, std::istringstream& stream)
{
    std::string buffer;

    if(read_file(path, buffer)) {
        stream.str(buffer);
        return true;
    }

    return false;
}

bool utils::write_file(std::string_view path, std::string_view buffer, bool append)
{
    auto path_unfucked = std::string(path);
    auto file = append ? PHYSFS_openAppend(path_unfucked.c_str()) : PHYSFS_openWrite(path_unfucked.c_str());

    if(file == nullptr) {
        LOG_WARNING("{}: {}", path_unfucked, physfs_error());
        return false;
    }

    PHYSFS_writeBytes(file, buffer.data(), buffer.size());
    PHYSFS_close(file);

    return true;
}

bool utils::write_file(std::string_view path, std::span<const std::byte> buffer, bool append)
{
    auto path_unfucked = std::string(path);
    auto file = append ? PHYSFS_openAppend(path_unfucked.c_str()) : PHYSFS_openWrite(path_unfucked.c_str());

    if(file == nullptr) {
        LOG_WARNING("{}: {}", path_unfucked, physfs_error());
        return false;
    }

    PHYSFS_writeBytes(file, buffer.data(), buffer.size());
    PHYSFS_close(file);

    return true;
}

std::string_view utils::physfs_error(void)
{
    return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
}
