#ifndef B5C8CD38_AF17_486A_B0E7_251BE96CAC86
#define B5C8CD38_AF17_486A_B0E7_251BE96CAC86

namespace config
{
template<typename T>
class Ref;
} // namespace config

namespace host
{
extern config::Ref<unsigned> max_players;
} // namespace host

namespace host
{
void init(void);
void init_late(void);
void shutdown(void);
void fixed_update_late(void);
} // namespace host

#endif /* B5C8CD38_AF17_486A_B0E7_251BE96CAC86 */
