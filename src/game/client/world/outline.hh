#pragma once

#include "shared/types.hh"

namespace outline
{
void init(void);
void shutdown(void);
void prepare(void);
} // namespace outline

namespace outline
{
void cube(const chunk_pos& cpos, const glm::fvec3& fpos, const glm::fvec3& size, float thickness, const glm::fvec4& color);
void line(const chunk_pos& cpos, const glm::fvec3& fpos, const glm::fvec3& size, float thickness, const glm::fvec4& color);
} // namespace outline
