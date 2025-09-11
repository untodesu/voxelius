#include "core/pch.hh"

#include "core/resource/binfile.hh"

#include "core/resource/resource.hh"

#include "core/utils/physfs.hh"

static const void* binfile_load_func(const char* name, std::uint32_t flags)
{
    assert(name);

    auto file = PHYSFS_openRead(name);

    if(file == nullptr) {
        spdlog::error("binfile: {}: {}", name, utils::physfs_error());
        return nullptr;
    }

    PHYSFS_sint64 file_size = PHYSFS_fileLength(file);

    if(file_size < 0) {
        spdlog::error("binfile: {}: {}", name, utils::physfs_error());
        PHYSFS_close(file);
        return nullptr;
    }

    auto binfile = new BinFile();
    binfile->size = static_cast<std::size_t>(file_size);
    binfile->buffer = new std::byte[binfile->size];

    PHYSFS_readBytes(file, binfile->buffer, file_size);
    PHYSFS_close(file);

    return binfile;
}

static void binfile_free_func(const void* resource)
{
    assert(resource);

    auto binfile = reinterpret_cast<const BinFile*>(resource);
    delete[] binfile->buffer;

    delete binfile;
}

void BinFile::register_resource(void)
{
    resource::register_loader<BinFile>(&binfile_load_func, &binfile_free_func);
}
