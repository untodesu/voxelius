#ifndef E1FF7339_64B7_4D12_827C_C538D783C402
#define E1FF7339_64B7_4D12_827C_C538D783C402

namespace utils
{
bool read_file(std::string_view path, std::string& buffer);
bool read_file(std::string_view path, std::vector<std::byte>& buffer);
bool read_file(std::string_view path, std::istringstream& stream);
} // namespace utils

namespace utils
{
bool write_file(std::string_view path, std::string_view buffer, bool append = false);
bool write_file(std::string_view path, std::span<const std::byte> buffer, bool append = false);
} // namespace utils

namespace utils
{
std::string_view physfs_error(void);
} // namespace utils

#endif /* E1FF7339_64B7_4D12_827C_C538D783C402 */
