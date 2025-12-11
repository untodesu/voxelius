#include "core/pch.hh"

#include "core/io/physfs.hh"

bool io::read_file(std::string_view path, std::vector<std::byte>& buffer)
{
    auto file = PHYSFS_openRead(std::string(path).c_str());

    if(file == nullptr) {
        spdlog::error("physfs: {}: {}", path, physfs_error());
        return false;
    }

    PHYSFS_sint64 file_size = PHYSFS_fileLength(file);
    buffer.resize(static_cast<std::size_t>(file_size));

    PHYSFS_readBytes(file, buffer.data(), file_size);
    PHYSFS_close(file);

    return true;
}

bool io::read_file(std::string_view path, std::string& buffer)
{
    auto file = PHYSFS_openRead(std::string(path).c_str());

    if(file == nullptr) {
        spdlog::error("physfs: {}: {}", path, physfs_error());
        return false;
    }

    PHYSFS_sint64 file_size = PHYSFS_fileLength(file);
    buffer.resize(static_cast<std::size_t>(file_size));

    PHYSFS_readBytes(file, buffer.data(), file_size);
    PHYSFS_close(file);

    return true;
}

bool io::write_file(std::string_view path, const std::vector<std::byte>& buffer)
{
    auto file = PHYSFS_openWrite(std::string(path).c_str());

    if(file == nullptr) {
        spdlog::error("physfs: {}: {}", path, physfs_error());
        return false;
    }

    PHYSFS_writeBytes(file, buffer.data(), static_cast<PHYSFS_uint64>(buffer.size()));
    PHYSFS_close(file);

    return true;
}

bool io::write_file(std::string_view path, const std::string& buffer)
{
    auto file = PHYSFS_openWrite(std::string(path).c_str());

    if(file == nullptr) {
        spdlog::error("physfs: {}: {}", path, physfs_error());
        return false;
    }

    PHYSFS_writeBytes(file, buffer.data(), static_cast<PHYSFS_uint64>(buffer.size()));
    PHYSFS_close(file);

    return true;
}

std::string_view io::physfs_error(void)
{
    auto error_code = PHYSFS_getLastErrorCode();
    auto error_string = PHYSFS_getErrorByCode(error_code);
    return std::string_view(error_string, std::strlen(error_string));
}
