#include "shared/pch.hh"
#include "shared/voxel_storage.hh"

#include "core/buffer.hh"

void VoxelStorage::serialize(WriteBuffer &buffer) const
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

    buffer.write_UI64(bound);

    // Write all the compressed data into the buffer
    for(std::size_t i = 0; i < bound; buffer.write_UI8(zdata[i++]));
}

void VoxelStorage::deserialize(ReadBuffer &buffer)
{
    auto size = static_cast<mz_ulong>(sizeof(VoxelStorage));
    auto bound = static_cast<mz_ulong>(buffer.read_UI64());
    auto zdata = std::vector<unsigned char>(bound);

    // Read all the compressed data from the buffer
    for(std::size_t i = 0; i < bound; zdata[i++] = buffer.read_UI8());

    mz_uncompress(reinterpret_cast<unsigned char*>(data()), &size, zdata.data(), bound);

    for(std::size_t i = 0; i < CHUNK_VOLUME; ++i) {
        // Convert voxel indices back into the host byte order
        at(i) = ENET_NET_TO_HOST_16(at(i));
    }
}
