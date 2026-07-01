#ifndef AAD525B4_8E79_498A_9DDA_A33EEED9EAA4
#define AAD525B4_8E79_498A_9DDA_A33EEED9EAA4

namespace constant
{
constexpr static std::size_t CHUNK_SIZE = 32;
constexpr static std::size_t CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;
constexpr static std::size_t CHUNK_VOLUME = CHUNK_AREA * CHUNK_SIZE;
constexpr static std::size_t CHUNK_SIZE_LOG2 = 5;
} // namespace constant

#endif /* AAD525B4_8E79_498A_9DDA_A33EEED9EAA4 */
