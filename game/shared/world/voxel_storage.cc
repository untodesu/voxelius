#include "shared/pch.hh"

#include "shared/world/voxel_storage.hh"

#include "core/io/buffer.hh"

void world::VoxelStorage::serialize(io::WriteBuffer& buffer) const
{
    auto bound = mz_compressBound(sizeof(VoxelStorage));
    auto zdata = std::vector<unsigned char>(bound);

    VoxelStorage net_storage;

    for(std::size_t i = 0; i < CHUNK_VOLUME; ++i) {
        // Convert voxel indices into network byte order;
        // We're going to compress them but we still want
        // the order to be consistent across all the platforms
        net_storage[i] = ENET_HOST_TO_NET_16(at(i));
    }

    mz_compress(zdata.data(), &bound, reinterpret_cast<unsigned char*>(net_storage.data()), sizeof(VoxelStorage));

    buffer.write<std::uint64_t>(bound);

    // Write all the compressed data into the buffer
    for(std::size_t i = 0; i < bound; buffer.write<std::uint8_t>(zdata[i++])) {
        // empty
    }
}

void world::VoxelStorage::deserialize(io::ReadBuffer& buffer)
{
    auto size = static_cast<mz_ulong>(sizeof(VoxelStorage));
    auto bound = static_cast<mz_ulong>(buffer.read<std::uint64_t>());
    auto zdata = std::vector<unsigned char>(bound);

    // Read all the compressed data from the buffer
    for(std::size_t i = 0; i < bound; zdata[i++] = buffer.read<std::uint8_t>()) {
        // empty
    }

    mz_uncompress(reinterpret_cast<unsigned char*>(data()), &size, zdata.data(), bound);

    for(std::size_t i = 0; i < CHUNK_VOLUME; ++i) {
        // Convert voxel indices back into the host byte order
        at(i) = ENET_NET_TO_HOST_16(at(i));
    }
}
