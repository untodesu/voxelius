#ifndef CORE_BINFILE_HH
#define CORE_BINFILE_HH 1
#pragma once

struct BinFile final {
    std::byte* buffer;
    std::size_t size;
};

#endif // CORE_BINFILE_HH
