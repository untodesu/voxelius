#ifndef B1F3492F_30A1_43E0_BD73_F36814AF8F72
#define B1F3492F_30A1_43E0_BD73_F36814AF8F72

#include "core/coord.hh"

namespace utils
{
ChunkPos to_chunk(const BlockPos& bpos) noexcept;
} // namespace utils

namespace utils
{
LocalPos to_local(const BlockPos& bpos) noexcept;
LocalPos to_local(std::size_t index) noexcept;
} // namespace utils

namespace utils
{
BlockPos to_block(const ChunkPos& cpos, const LocalPos& lpos) noexcept;
} // namespace utils

namespace utils
{
std::size_t to_index(const LocalPos& lpos) noexcept;
} // namespace utils

namespace utils
{
Eigen::Vector3f to_relative(const ChunkPos& pivot_cpos, const ChunkPos& cpos, const Eigen::Vector3f& fvec) noexcept;
Eigen::Vector3f to_relative(const ChunkPos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const ChunkPos& cpos) noexcept;
Eigen::Vector3f to_relative(const ChunkPos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const ChunkPos& cpos,
    const Eigen::Vector3f& fvec) noexcept;
} // namespace utils

namespace utils
{
Eigen::Vector3f to_fvec(const ChunkPos& cpos) noexcept;
Eigen::Vector3f to_fvec(const ChunkPos& cpos, const Eigen::Vector3f& offset) noexcept;
} // namespace utils

#endif /* B1F3492F_30A1_43E0_BD73_F36814AF8F72 */
