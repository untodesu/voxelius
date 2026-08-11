#ifndef E5FBE30D_A589_4737_8B37_7DE782E75B94
#define E5FBE30D_A589_4737_8B37_7DE782E75B94

namespace universe
{
void init(void);
void init_late(void);
void shutdown(void);
} // namespace universe

namespace universe
{
const std::filesystem::path& chunk_dir(void);
const std::filesystem::path& entity_dir(void);
std::uint64_t seed(void);
} // namespace universe

#endif /* E5FBE30D_A589_4737_8B37_7DE782E75B94 */
