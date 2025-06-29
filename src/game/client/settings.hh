#ifndef CLIENT_SETTINGS_HH
#define CLIENT_SETTINGS_HH 1
#pragma once

class ConfigBoolean;
class ConfigString;

class ConfigInt;
class ConfigFloat;
class ConfigUnsigned;

class ConfigKeyBind;
class ConfigGamepadAxis;
class ConfigGamepadButton;

enum class settings_location : unsigned int {
    GENERAL = 0x0000U,
    KEYBOARD_MOVEMENT = 0x0001U,
    KEYBOARD_GAMEPLAY = 0x0002U,
    KEYBOARD_MISC = 0x0003U,
    GAMEPAD = 0x0004U,
    GAMEPAD_MOVEMENT = 0x0005U,
    GAMEPAD_GAMEPLAY = 0x0006U,
    GAMEPAD_MISC = 0x0007U,
    MOUSE = 0x0008U,
    VIDEO = 0x0009U,
    VIDEO_GUI = 0x000AU,
    SOUND = 0x000BU,
    SOUND_LEVELS = 0x000CU,
    COUNT = 0x000DU,
};

namespace settings
{
void init(void);
void init_late(void);
void deinit(void);
void layout(void);
} // namespace settings

namespace settings
{
void add_checkbox(int priority, ConfigBoolean& value, settings_location location, const char* name, bool tooltip);
} // namespace settings

namespace settings
{
void add_input(int priority, ConfigInt& value, settings_location location, const char* name, bool tooltip);
void add_input(int priority, ConfigFloat& value, settings_location location, const char* name, bool tooltip, const char* format = "%.3f");
void add_input(int priority, ConfigUnsigned& value, settings_location location, const char* name, bool tooltip);
void add_input(int priority, ConfigString& value, settings_location location, const char* name, bool tooltip, bool allow_whitespace);
} // namespace settings

namespace settings
{
void add_slider(int priority, ConfigInt& value, settings_location location, const char* name, bool tooltip);
void add_slider(int priority, ConfigFloat& value, settings_location location, const char* name, bool tooltip, const char* format = "%.3f");
void add_slider(int priority, ConfigUnsigned& value, settings_location location, const char* name, bool tooltip);
} // namespace settings

namespace settings
{
void add_stepper(int priority, ConfigInt& value, settings_location location, const char* name, bool tooltip);
void add_stepper(int priority, ConfigUnsigned& value, settings_location location, const char* name, bool tooltip);
} // namespace settings

namespace settings
{
void add_keybind(int priority, ConfigKeyBind& value, settings_location location, const char* name);
} // namespace settings

namespace settings
{
void add_gamepad_axis(int priority, ConfigGamepadAxis& value, settings_location location, const char* name);
void add_gamepad_button(int priority, ConfigGamepadButton& value, settings_location location, const char* name);
} // namespace settings

namespace settings
{
void add_language_select(int priority, settings_location location, const char* name);
} // namespace settings

#endif // CLIENT_SETTINGS_HH
