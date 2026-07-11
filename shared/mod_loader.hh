#ifndef C7905765_3FDE_4BA6_BF70_AEC7A901090D
#define C7905765_3FDE_4BA6_BF70_AEC7A901090D

#include "shared/mod_context.hh"

namespace mod_loader
{
void init(void);
void shutdown(void);
} // namespace mod_loader

namespace mod_loader
{
std::span<const ModContext> all(void);
} // namespace mod_loader

namespace mod_loader
{
const ModContext* find(std::string_view name_space);
} // namespace mod_loader

#endif /* C7905765_3FDE_4BA6_BF70_AEC7A901090D */
