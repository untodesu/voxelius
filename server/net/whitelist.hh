#ifndef B969E03D_5FD1_4185_AE9C_301DACEEA895
#define B969E03D_5FD1_4185_AE9C_301DACEEA895

#include "shared/net/ed25519.hh"

namespace whitelist
{
void init(void);
void shutdown(void);
} // namespace whitelist

namespace whitelist
{
void add(const ed25519::pkey_type& pkey);
void remove(const ed25519::pkey_type& pkey);
bool contains(const ed25519::pkey_type& pkey);
} // namespace whitelist

#endif /* B969E03D_5FD1_4185_AE9C_301DACEEA895 */
