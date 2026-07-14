#ifndef C0A5709C_A57B_4D67_AC7A_4FA42E2EEAFC
#define C0A5709C_A57B_4D67_AC7A_4FA42E2EEAFC

#include "shared/coord.hh"

namespace outline
{
void init(void);
void shutdown(void);
void prepare(void);
} // namespace outline

namespace outline
{
void cube(const ChunkPos& cpos, const Eigen::Vector3f& fpos, const Eigen::Vector3f& size, float thickness,
    const Eigen::Vector4f& color) noexcept;
void line(const ChunkPos& cpos, const Eigen::Vector3f& fpos, const Eigen::Vector3f& size, float thickness,
    const Eigen::Vector4f& color) noexcept;
} // namespace outline

#endif /* C0A5709C_A57B_4D67_AC7A_4FA42E2EEAFC */
