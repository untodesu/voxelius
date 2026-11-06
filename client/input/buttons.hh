#ifndef EFE08C04_383B_46E7_95DD_5781E755DCF8
#define EFE08C04_383B_46E7_95DD_5781E755DCF8

#include "client/input/source.hh"

namespace buttons
{
extern bool is_capturing;
} // namespace buttons

namespace buttons
{
void init(void);
} // namespace buttons

namespace buttons
{
void bind(ButtonSource source, std::string command, bool overwrite = true);
void unbind(ButtonSource source);
void clear(void);
} // namespace buttons

namespace buttons
{
std::string_view find_command(ButtonSource source);
std::vector<ButtonSource> find(std::string_view command);
} // namespace buttons

#endif /* EFE08C04_383B_46E7_95DD_5781E755DCF8 */
