#ifndef CA457813_A93D_4AE7_8644_4B9FECFCD782
#define CA457813_A93D_4AE7_8644_4B9FECFCD782

#include "core/constant.hh"

namespace constant
{
constexpr static std::size_t CHUNK_SIZE = 32;
constexpr static std::size_t CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;
constexpr static std::size_t CHUNK_VOLUME = CHUNK_AREA * CHUNK_SIZE;
constexpr static std::size_t CHUNK_SIZE_LOG2 = 5;
} // namespace constant

#endif /* CA457813_A93D_4AE7_8644_4B9FECFCD782 */
