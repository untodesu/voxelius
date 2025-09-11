#pragma once

namespace math
{
std::uint64_t crc64(const void* buffer, std::size_t size, std::uint64_t combine = UINT64_C(0));
std::uint64_t crc64(const std::vector<std::byte>& buffer, std::uint64_t combine = UINT64_C(0));
std::uint64_t crc64(const std::string& buffer, std::uint64_t combine = UINT64_C(0));
} // namespace math
