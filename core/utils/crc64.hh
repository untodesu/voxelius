#ifndef BDEB361F_BB84_42B0_9FD8_47D602A52945
#define BDEB361F_BB84_42B0_9FD8_47D602A52945

namespace utils
{
std::uint64_t crc64(const void* buffer, std::size_t size, std::uint64_t combine = 0);
std::uint64_t crc64(std::span<const std::byte> buffer, std::uint64_t combine = 0);
} // namespace utils

namespace utils
{
template<typename T>
std::uint64_t crc64(std::span<const T> buffer, std::uint64_t combine = 0);
} // namespace utils

template<typename T>
std::uint64_t utils::crc64(std::span<const T> buffer, std::uint64_t combine)
{
    return crc64(std::as_bytes(buffer), combine);
}

#endif /* BDEB361F_BB84_42B0_9FD8_47D602A52945 */
