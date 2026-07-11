#ifndef B1F3492F_30A1_43E0_BD73_F36814AF8F72
#define B1F3492F_30A1_43E0_BD73_F36814AF8F72

#include "shared/coord.hh"

namespace utils
{
chunk_pos to_chunk(const block_pos& bpos);
} // namespace utils

namespace utils
{
local_pos to_local(const block_pos& bpos);
local_pos to_local(std::size_t index);
} // namespace utils

namespace utils
{
block_pos to_block(const chunk_pos& cpos, const local_pos& lpos);
} // namespace utils

namespace utils
{
std::size_t to_index(const local_pos& lpos);
} // namespace utils

namespace utils
{
Eigen::Vector3f to_relative(const chunk_pos& pivot_cpos, const chunk_pos& cpos, const Eigen::Vector3f& fvec);
Eigen::Vector3f to_relative(const chunk_pos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const chunk_pos& cpos);
Eigen::Vector3f to_relative(const chunk_pos& pivot_cpos, const Eigen::Vector3f& pivot_fvec, const chunk_pos& cpos,
    const Eigen::Vector3f& fvec);
} // namespace utils

namespace utils
{
Eigen::Vector3f to_fvec(const chunk_pos& cpos);
Eigen::Vector3f to_fvec(const chunk_pos& cpos, const Eigen::Vector3f& offset);
} // namespace utils

namespace utils
{
local_pos wrap_local(const local_pos& lpos);
} // namespace utils

#endif /* B1F3492F_30A1_43E0_BD73_F36814AF8F72 */
