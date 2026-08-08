#ifndef F6A96927_C0AA_4818_B3E8_D28008536EAD
#define F6A96927_C0AA_4818_B3E8_D28008536EAD

namespace ed25519
{
using seed_type = std::array<std::byte, 32>;
using pkey_type = std::array<std::byte, 32>;
using skey_type = std::array<std::byte, 64>;
using exch_type = std::array<std::byte, 32>;
using sign_type = std::array<std::byte, 64>;
} // namespace ed25519

namespace ed25519
{
using pair_type = std::pair<pkey_type, skey_type>;
} // namespace ed25519

namespace ed25519
{
seed_type generate_seed(void);
pair_type generate_pair(const seed_type& seed);
exch_type generate_exch(const skey_type& a_skey, const pkey_type& b_pkey);
} // namespace ed25519

namespace ed25519
{
sign_type sign(const pair_type& pair, std::span<const std::byte> mesg);
} // namespace ed25519

namespace ed25519
{
bool verify(const pkey_type& pkey, std::span<const std::byte> mesg, const sign_type& sign);
} // namespace ed25519

namespace ed25519
{
std::optional<pkey_type> import_pkey(std::string_view xstr);
std::optional<skey_type> import_skey(std::string_view xstr);
} // namespace ed25519

namespace ed25519
{
std::string export_pkey(const pkey_type& pkey);
std::string export_skey(const skey_type& skey);
std::string export_sign(const sign_type& sign);
} // namespace ed25519

#endif /* F6A96927_C0AA_4818_B3E8_D28008536EAD */
