#ifndef B1F3492F_30A1_43E0_BD73_F36814AF8F72
#define B1F3492F_30A1_43E0_BD73_F36814AF8F72

#include "shared/coord.hh"

namespace utils
{
ChunkPos to_chunk(const BlockPos& bpos);
} // namespace utils

namespace utils
{
LocalPos to_local(const BlockPos& bpos);
LocalPos to_local(std::size_t index);
} // namespace utils

namespace utils
{
BlockPos to_block(const ChunkPos& cpos, const LocalPos& lpos);
} // namespace utils

namespace utils
{
std::size_t to_index(const LocalPos& lpos);
} // namespace utils

namespace utils
{
Eigen::Vector3f to_relative(const ChunkPos& pivot_cpos, const ChunkPos& cpos, const Eigen::Vector3f& fvec);
Eigen::Vector3f to_relative(const ChunkPos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const ChunkPos& cpos);
Eigen::Vector3f to_relative(const ChunkPos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const ChunkPos& cpos,
    const Eigen::Vector3f& fvec);
} // namespace utils

namespace utils
{
Eigen::Vector3f to_fvec(const ChunkPos& cpos);
Eigen::Vector3f to_fvec(const ChunkPos& cpos, const Eigen::Vector3f& offset);
} // namespace utils

namespace utils
{
LocalPos wrap_local(const LocalPos& lpos);
} // namespace utils

#endif /* B1F3492F_30A1_43E0_BD73_F36814AF8F72 */
