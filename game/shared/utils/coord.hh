#ifndef B1F3492F_30A1_43E0_BD73_F36814AF8F72
#define B1F3492F_30A1_43E0_BD73_F36814AF8F72

#include "game/shared/coord.hh"

namespace utils
{
chunk_pos to_chunk(const block_pos& bpos) noexcept;
} // namespace utils

namespace utils
{
local_pos to_local(const block_pos& bpos) noexcept;
local_pos to_local(std::size_t index) noexcept;
} // namespace utils

namespace utils
{
block_pos to_block(const chunk_pos& cpos, const local_pos& lpos) noexcept;
} // namespace utils

namespace utils
{
std::size_t to_index(const local_pos& lpos) noexcept;
} // namespace utils

namespace utils
{
Eigen::Vector3f to_relative(const chunk_pos& pivot_cpos, const chunk_pos& cpos, const Eigen::Vector3f& fvec) noexcept;
Eigen::Vector3f to_relative(const chunk_pos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const chunk_pos& cpos) noexcept;
Eigen::Vector3f to_relative(const chunk_pos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const chunk_pos& cpos,
    const Eigen::Vector3f& fvec) noexcept;
} // namespace utils

namespace utils
{
Eigen::Vector3f to_fvec(const chunk_pos& cpos) noexcept;
Eigen::Vector3f to_fvec(const chunk_pos& cpos, const Eigen::Vector3f& offset) noexcept;
} // namespace utils

#endif /* B1F3492F_30A1_43E0_BD73_F36814AF8F72 */
