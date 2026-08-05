#ifndef B5AE2945_F62A_491D_B0D1_943D061F008C
#define B5AE2945_F62A_491D_B0D1_943D061F008C

#include "shared/entity/class.hh"

// The engine hard-depends on some classes that
// Lua may or may not define. This subsystem keeps
// track of these classes and ensures they are
// present after all the mods have finished loading

class ModContext;

namespace required_class
{
extern class_id_type player;
} // namespace required_class

namespace required_class
{
void set_player(const Identifier& id, const ModContext* ctx);
} // namespace required_class

namespace required_class
{
void resolve(void);
} // namespace required_class

#endif /* B5AE2945_F62A_491D_B0D1_943D061F008C */
