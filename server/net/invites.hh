#ifndef D80F2C60_9B5E_4D84_B3CD_CED84D7223FF
#define D80F2C60_9B5E_4D84_B3CD_CED84D7223FF

#include "shared/net/ed25519.hh"

namespace invites
{
void init(void);
void shutdown(void);
} // namespace invites

namespace invites
{
std::uint64_t issue(void);
} // namespace invites

namespace invites
{
bool consume(std::uint64_t token, const ed25519::pkey_type& pkey);
} // namespace invites

#endif /* D80F2C60_9B5E_4D84_B3CD_CED84D7223FF */
