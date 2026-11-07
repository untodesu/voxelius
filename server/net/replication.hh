#ifndef BDA919B9_3638_4D28_83FE_1FB1D4E89698
#define BDA919B9_3638_4D28_83FE_1FB1D4E89698

struct Session;

namespace replication
{
void init(void);
void init_late(void);
void fixed_update_late(void);
} // namespace replication

namespace replication
{
void synchronize(Session* session);
} // namespace replication

#endif /* BDA919B9_3638_4D28_83FE_1FB1D4E89698 */
