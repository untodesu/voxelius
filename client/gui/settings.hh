#ifndef B785A84D_AF42_43DC_A513_BF3003C221DD
#define B785A84D_AF42_43DC_A513_BF3003C221DD

#include "core/concepts.hh"

enum class settings_location {
    GENERAL = 0,
    KEYBOARD_MOVEMENT,
    KEYBOARD_GAMEPLAY,
    KEYBOARD_MISC,
    GAMEPAD,
    GAMEPAD_MOVEMENT,
    GAMEPAD_GAMEPLAY,
    GAMEPAD_MISC,
    MOUSE,
    VIDEO,
    VIDEO_GUI,
    SOUND,
    SOUND_LEVELS,
    count
};

namespace settings
{
void init(void);
void init_late(void);
void shutdown(void);
void layout(void);
} // namespace settings

namespace settings
{
void checkbox(int priority, settings_location location, std::string_view key, bool tooltip);
} // namespace settings

namespace settings
{
template<vx::arithmetic T>
void input_arithmetic(int priority, settings_location location, std::string_view key, T min, T max, bool tooltip,
    std::string_view fmt = {});
void input_string(int priority, settings_location location, std::string_view key, bool tooltip, bool allow_whitespace);
} // namespace settings

namespace settings
{
template<vx::arithmetic T>
void slider(int priority, settings_location location, std::string_view key, T min, T max, bool tooltip, std::string_view fmt = {});
} // namespace settings

namespace settings
{
template<std::integral T>
void stepper(int priority, settings_location location, std::string_view key, T min, T max, T step, bool tooltip);
} // namespace settings

namespace settings
{
void keybind(int priority, settings_location location, std::string_view key, bool tooltip);
void language(int priority, settings_location location, std::string_view key, bool tooltip);
void video_mode(int priority, settings_location location, std::string_view key, bool tooltip);
} // namespace settings

#endif /* B785A84D_AF42_43DC_A513_BF3003C221DD */
