#ifndef CORE_RESOURCE_BINFILE_HH
#define CORE_RESOURCE_BINFILE_HH 1
#pragma once

struct BinFile final {
    static void register_resource(void);

    std::byte* buffer;
    std::size_t size;
};

#endif // CORE_RESOURCE_BINFILE_HH
