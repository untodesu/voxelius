#include "client/pch.hh"

#include "client/config/keybind.hh"

#include "core/math/constexpr.hh"

#include "client/const.hh"

constexpr static std::string_view UNKNOWN_KEY_NAME = "UNKNOWN";

static const std::pair<int, std::string_view> key_names[] = {
    { GLFW_KEY_SPACE, "SPACE" },
    { GLFW_KEY_APOSTROPHE, "'" },
    { GLFW_KEY_COMMA, "," },
    { GLFW_KEY_MINUS, "-" },
    { GLFW_KEY_PERIOD, "." },
    { GLFW_KEY_SLASH, "/" },
    { GLFW_KEY_0, "0" },
    { GLFW_KEY_1, "1" },
    { GLFW_KEY_2, "2" },
    { GLFW_KEY_3, "3" },
    { GLFW_KEY_4, "4" },
    { GLFW_KEY_5, "5" },
    { GLFW_KEY_6, "6" },
    { GLFW_KEY_7, "7" },
    { GLFW_KEY_8, "8" },
    { GLFW_KEY_9, "9" },
    { GLFW_KEY_SEMICOLON, ";" },
    { GLFW_KEY_EQUAL, "=" },
    { GLFW_KEY_A, "A" },
    { GLFW_KEY_B, "B" },
    { GLFW_KEY_C, "C" },
    { GLFW_KEY_D, "D" },
    { GLFW_KEY_E, "E" },
    { GLFW_KEY_F, "F" },
    { GLFW_KEY_G, "G" },
    { GLFW_KEY_H, "H" },
    { GLFW_KEY_I, "I" },
    { GLFW_KEY_J, "J" },
    { GLFW_KEY_K, "K" },
    { GLFW_KEY_L, "L" },
    { GLFW_KEY_M, "M" },
    { GLFW_KEY_N, "N" },
    { GLFW_KEY_O, "O" },
    { GLFW_KEY_P, "P" },
    { GLFW_KEY_Q, "Q" },
    { GLFW_KEY_R, "R" },
    { GLFW_KEY_S, "S" },
    { GLFW_KEY_T, "T" },
    { GLFW_KEY_U, "U" },
    { GLFW_KEY_V, "V" },
    { GLFW_KEY_W, "W" },
    { GLFW_KEY_X, "X" },
    { GLFW_KEY_Y, "Y" },
    { GLFW_KEY_Z, "Z" },
    { GLFW_KEY_LEFT_BRACKET, "[" },
    { GLFW_KEY_BACKSLASH, "\\" },
    { GLFW_KEY_RIGHT_BRACKET, "]" },
    { GLFW_KEY_GRAVE_ACCENT, "`" },
    { GLFW_KEY_WORLD_1, "WORLD_1" },
    { GLFW_KEY_WORLD_2, "WORLD_2" },
    { GLFW_KEY_ESCAPE, "ESCAPE" },
    { GLFW_KEY_ENTER, "ENTER" },
    { GLFW_KEY_TAB, "TAB" },
    { GLFW_KEY_BACKSPACE, "BACKSPACE" },
    { GLFW_KEY_INSERT, "INSERT" },
    { GLFW_KEY_DELETE, "DELETE" },
    { GLFW_KEY_RIGHT, "RIGHT" },
    { GLFW_KEY_LEFT, "LEFT" },
    { GLFW_KEY_DOWN, "DOWN" },
    { GLFW_KEY_UP, "UP" },
    { GLFW_KEY_PAGE_UP, "PAGE_UP" },
    { GLFW_KEY_PAGE_DOWN, "PAGE_DOWN" },
    { GLFW_KEY_HOME, "HOME" },
    { GLFW_KEY_END, "END" },
    { GLFW_KEY_CAPS_LOCK, "CAPS_LOCK" },
    { GLFW_KEY_SCROLL_LOCK, "SCROLL_LOCK" },
    { GLFW_KEY_NUM_LOCK, "NUM_LOCK" },
    { GLFW_KEY_PRINT_SCREEN, "PRINT_SCREEN" },
    { GLFW_KEY_PAUSE, "PAUSE" },
    { GLFW_KEY_F1, "F1" },
    { GLFW_KEY_F2, "F2" },
    { GLFW_KEY_F3, "F3" },
    { GLFW_KEY_F4, "F4" },
    { GLFW_KEY_F5, "F5" },
    { GLFW_KEY_F6, "F6" },
    { GLFW_KEY_F7, "F7" },
    { GLFW_KEY_F8, "F8" },
    { GLFW_KEY_F9, "F9" },
    { GLFW_KEY_F10, "F10" },
    { GLFW_KEY_F11, "F11" },
    { GLFW_KEY_F12, "F12" },
    { GLFW_KEY_F13, "F13" },
    { GLFW_KEY_F14, "F14" },
    { GLFW_KEY_F15, "F15" },
    { GLFW_KEY_F16, "F16" },
    { GLFW_KEY_F17, "F17" },
    { GLFW_KEY_F18, "F18" },
    { GLFW_KEY_F19, "F19" },
    { GLFW_KEY_F20, "F20" },
    { GLFW_KEY_F21, "F21" },
    { GLFW_KEY_F22, "F22" },
    { GLFW_KEY_F23, "F23" },
    { GLFW_KEY_F24, "F24" },
    { GLFW_KEY_F25, "F25" },
    { GLFW_KEY_KP_0, "KEYPAD_0" },
    { GLFW_KEY_KP_1, "KEYPAD_1" },
    { GLFW_KEY_KP_2, "KEYPAD_2" },
    { GLFW_KEY_KP_3, "KEYPAD_3" },
    { GLFW_KEY_KP_4, "KEYPAD_4" },
    { GLFW_KEY_KP_5, "KEYPAD_5" },
    { GLFW_KEY_KP_6, "KEYPAD_6" },
    { GLFW_KEY_KP_7, "KEYPAD_7" },
    { GLFW_KEY_KP_8, "KEYPAD_8" },
    { GLFW_KEY_KP_9, "KEYPAD_9" },
    { GLFW_KEY_KP_DECIMAL, "KEYPAD_POINT" },
    { GLFW_KEY_KP_DIVIDE, "KEYPAD_DIV" },
    { GLFW_KEY_KP_MULTIPLY, "KEYPAD_MUL" },
    { GLFW_KEY_KP_SUBTRACT, "KEYPAD_MINUS" },
    { GLFW_KEY_KP_ADD, "KEYPAD_PLUS" },
    { GLFW_KEY_KP_ENTER, "KEYPAD_ENTER" },
    { GLFW_KEY_KP_EQUAL, "KEYPAD_EQUAL" },
    { GLFW_KEY_LEFT_SHIFT, "LEFT_SHIFT" },
    { GLFW_KEY_LEFT_CONTROL, "LEFT_CTRL" },
    { GLFW_KEY_LEFT_ALT, "LEFT_ALT" },
    { GLFW_KEY_LEFT_SUPER, "LEFT_SUPER" },
    { GLFW_KEY_RIGHT_SHIFT, "RIGHT_SHIFT" },
    { GLFW_KEY_RIGHT_CONTROL, "RIGHT_CTRL" },
    { GLFW_KEY_RIGHT_ALT, "RIGHT_ALT" },
    { GLFW_KEY_RIGHT_SUPER, "RIGHT_SUPER" },
    { GLFW_KEY_MENU, "MENU" },
};

static std::string_view get_key_name(int keycode)
{
    for(const auto& it : key_names) {
        if(it.first == keycode) {
            return it.second;
        }
    }

    return UNKNOWN_KEY_NAME;
}

config::KeyBind::KeyBind(void)
{
    m_glfw_keycode = GLFW_KEY_UNKNOWN;
    m_name = UNKNOWN_KEY_NAME;
}

config::KeyBind::KeyBind(int default_value)
{
    if(default_value == DEBUG_KEY) {
        m_glfw_keycode = GLFW_KEY_UNKNOWN;
        m_name = UNKNOWN_KEY_NAME;
    }
    else {
        m_glfw_keycode = default_value;
        m_name = get_key_name(default_value);
    }
}

void config::KeyBind::set(std::string_view value)
{
    for(const auto& it : key_names) {
        if((it.first != DEBUG_KEY) && 0 == it.second.compare(value)) {
            m_glfw_keycode = it.first;
            m_name = it.second;
            return;
        }
    }

    m_glfw_keycode = GLFW_KEY_UNKNOWN;
    m_name = UNKNOWN_KEY_NAME;
}

std::string_view config::KeyBind::get(void) const
{
    return m_name;
}

void config::KeyBind::set_key(int keycode)
{
    if(keycode == DEBUG_KEY) {
        m_glfw_keycode = GLFW_KEY_UNKNOWN;
        m_name = UNKNOWN_KEY_NAME;
    }
    else {
        m_glfw_keycode = keycode;
        m_name = get_key_name(keycode);
    }
}

int config::KeyBind::get_key(void) const
{
    return m_glfw_keycode;
}

bool config::KeyBind::equals(int keycode) const
{
    return m_glfw_keycode == keycode;
}
