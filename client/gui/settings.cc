#include "client/pch.hh"

#include "client/gui/settings.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;
constexpr static unsigned int NUM_LOCATIONS = static_cast<unsigned int>(settings_location::count);
constexpr static std::string_view TOOLTIP_TEXT = "[?]";

void settings::init(void)
{
}

void settings::init_late(void)
{
}

void settings::shutdown(void)
{
}

void settings::layout(void)
{
}

void settings::checkbox(int priority, std::string_view key, bool tooltip)
{
}

template<vx::arithmetic T>
void settings::input_arithmetic(int priority, std::string_view key, T min, T max, bool tooltip, std::string_view fmt)
{
}

template void settings::input_arithmetic<float>(int priority, std::string_view key, float min, float max, bool tooltip,
    std::string_view fmt);
template void settings::input_arithmetic<unsigned>(int priority, std::string_view key, unsigned min, unsigned max, bool tooltip,
    std::string_view fmt);
template void settings::input_arithmetic<int>(int priority, std::string_view key, int min, int max, bool tooltip, std::string_view fmt);

void settings::input_string(int priority, std::string_view key, bool tooltip, bool allow_whitespace)
{
}

template<vx::arithmetic T>
void settings::slider(int priority, std::string_view key, T min, T max, bool tooltip, std::string_view fmt)
{
}

template void settings::slider<float>(int priority, std::string_view key, float min, float max, bool tooltip, std::string_view fmt);
template void settings::slider<unsigned>(int priority, std::string_view key, unsigned min, unsigned max, bool tooltip,
    std::string_view fmt);
template void settings::slider<int>(int priority, std::string_view key, int min, int max, bool tooltip, std::string_view fmt);

template<std::integral T>
void settings::stepper(int priority, std::string_view key, T min, T max, T step, bool tooltip)
{
}

template void settings::stepper<unsigned>(int priority, std::string_view key, unsigned min, unsigned max, unsigned step, bool tooltip);
template void settings::stepper<int>(int priority, std::string_view key, int min, int max, int step, bool tooltip);

void settings::keybind(int priority, std::string_view key, bool tooltip)
{
}

void settings::language(int priority, std::string_view key, bool tooltip)
{
}

void settings::video_mode(int priority, std::string_view key, bool tooltip)
{
}
