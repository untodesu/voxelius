#include "core/pch.hh"

#include "core/utils/enet.hh"

constexpr static int LZ_ACCELERATION = 1;

struct CompressionContext final {
    std::vector<std::byte> state;
    std::array<char, ENET_PROTOCOL_MAXIMUM_MTU> gather;
};

static std::size_t compress_packet(void* context, const ENetBuffer* in_buffers, std::size_t in_buffer_count, std::size_t in_limit,
    enet_uint8* out_data, std::size_t out_limit)
{
    auto self = reinterpret_cast<CompressionContext*>(context);

    if(in_limit > self->gather.size() || in_limit > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return 0;
    }

    if(out_limit == 0 || out_limit > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return 0;
    }

    std::size_t gathered = 0;

    for(std::size_t i = 0; i < in_buffer_count; ++i) {
        if(gathered + in_buffers[i].dataLength > in_limit) {
            return 0;
        }

        std::memcpy(&self->gather[gathered], in_buffers[i].data, in_buffers[i].dataLength);

        gathered += in_buffers[i].dataLength;
    }

    if(gathered == 0) {
        return 0;
    }

    auto src_size_lz = static_cast<int>(gathered);
    auto dst_size_lz = static_cast<int>(out_limit);
    auto dst_lz = reinterpret_cast<char*>(out_data);
    auto size = LZ4_compress_fast_extState(self->state.data(), self->gather.data(), dst_lz, src_size_lz, dst_size_lz, LZ_ACCELERATION);

    if(size > 0) {
        return static_cast<std::size_t>(size);
    }
    else {
        return 0;
    }
}

static std::size_t decompress_packet(void* context, const enet_uint8* in_data, std::size_t in_limit, enet_uint8* out_data,
    std::size_t out_limit)
{
    if(in_limit == 0 || in_limit > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return 0;
    }

    if(out_limit == 0 || out_limit > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return 0;
    }

    auto src_lz = reinterpret_cast<const char*>(in_data);
    auto dst_lz = reinterpret_cast<char*>(out_data);
    auto src_size_lz = static_cast<int>(in_limit);
    auto dst_size_lz = static_cast<int>(out_limit);
    auto size = LZ4_decompress_safe(src_lz, dst_lz, src_size_lz, dst_size_lz);

    if(size > 0) {
        return static_cast<std::size_t>(size);
    }
    else {
        return 0;
    }
}

static void destroy_packet(void* context)
{
    delete reinterpret_cast<CompressionContext*>(context);
}

void utils::enable_compression(ENetHost* host)
{
    assert(host);

    auto context = new CompressionContext {};
    context->state.resize(static_cast<std::size_t>(LZ4_sizeofState()));

    ENetCompressor compressor {};
    compressor.context = context;
    compressor.compress = &compress_packet;
    compressor.decompress = &decompress_packet;
    compressor.destroy = &destroy_packet;

    enet_host_compress(host, &compressor);
}
