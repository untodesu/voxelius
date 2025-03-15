#ifndef CORE_CRC64_HH
#define CORE_CRC64_HH 1
#pragma once

namespace crc64
{
std::uint64_t get(const void *buffer, std::size_t size, std::uint64_t combine = UINT64_C(0));
std::uint64_t get(const std::vector<std::byte> &buffer, std::uint64_t combine = UINT64_C(0));
std::uint64_t get(const std::string &buffer, std::uint64_t combine = UINT64_C(0));
} // namespace crc64

#endif /* CORE_CRC64_HH */
