// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: settings.cc
// Description: Visual config value editing

#include "client/pch.hh"

#include "client/gui/settings.hh"

#include "core/config/boolean.hh"
#include "core/config/number.hh"
#include "core/config/string.hh"

#include "core/io/config_map.hh"

#include "core/math/constexpr.hh"

#include "client/config/gamepad_axis.hh"
#include "client/config/gamepad_button.hh"
#include "client/config/keybind.hh"

#include "client/gui/gui_screen.hh"
#include "client/gui/imutils_button.hh"
#include "client/gui/imutils_popup.hh"
#include "client/gui/language.hh"

#include "client/io/gamepad.hh"
#include "client/io/keyboard.hh"
#include "client/io/video.hh"

#include "client/const.hh"
#include "client/globals.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;
constexpr static unsigned int NUM_LOCATIONS = static_cast<unsigned int>(settings_location::COUNT);
constexpr static const char* TOOLTIP_TEXT = "[?]";

enum class setting_type {
    CHECKBOX,          ///< config::Boolean
    INPUT_INT,         ///< config::Number<int>
    INPUT_FLOAT,       ///< config::Number<float>
    INPUT_UINT,        ///< config::Number<unsigned int>
    INPUT_STRING,      ///< config::String
    SLIDER_INT,        ///< config::Number<int>
    SLIDER_FLOAT,      ///< config::Number<float>
    SLIDER_UINT,       ///< config::Number<unsigned int>
    STEPPER_INT,       ///< config::Number<int>
    STEPPER_UINT,      ///< config::Number<unsigned int>
    KEYBIND,           ///< config::KeyBind
    GAMEPAD_AXIS,      ///< config::GamepadAxis
    GAMEPAD_BUTTON,    ///< config::GamepadButton
    LANGUAGE_SELECT,   ///< config::String internally
    VIDEO_MODE_SELECT, ///< config::String internally
};

enum class gui_location {
    GENERAL,
    BINDINGS,
    AUDIO,
    VIDEO,
    KEYBOARD,
    MOUSE,
    GAMEPAD,
};

class SettingValue {
public:
    virtual ~SettingValue(void) = default;
    virtual void layout(void) const = 0;
    void layout_tooltip(void) const;
    void layout_label(void) const;

public:
    setting_type type;
    std::string tooltip;
    std::string title;
    std::string name;
    bool has_tooltip;
    int priority;
};

class SettingValueWID : public SettingValue {
public:
    virtual ~SettingValueWID(void) = default;

public:
    std::string wid;
};

class SettingValue_CheckBox final : public SettingValue {
public:
    virtual ~SettingValue_CheckBox(void) = default;
    virtual void layout(void) const override;
    void refresh_wids(void);

public:
    config::Boolean* value;
    std::string wids[2];
};

class SettingValue_InputInt final : public SettingValueWID {
public:
    virtual ~SettingValue_InputInt(void) = default;
    virtual void layout(void) const override;

public:
    config::Int* value;
};

class SettingValue_InputFloat final : public SettingValueWID {
public:
    virtual ~SettingValue_InputFloat(void) = default;
    virtual void layout(void) const override;

public:
    std::string format;
    config::Float* value;
};

class SettingValue_InputUnsigned final : public SettingValueWID {
public:
    virtual ~SettingValue_InputUnsigned(void) = default;
    virtual void layout(void) const override;

public:
    config::Unsigned* value;
};

class SettingValue_InputString final : public SettingValueWID {
public:
    virtual ~SettingValue_InputString(void) = default;
    virtual void layout(void) const override;

public:
    config::String* value;
    bool allow_whitespace;
};

class SettingValue_SliderInt final : public SettingValueWID {
public:
    virtual ~SettingValue_SliderInt(void) = default;
    virtual void layout(void) const override;

public:
    config::Int* value;
};

class SettingValue_SliderFloat final : public SettingValueWID {
public:
    virtual ~SettingValue_SliderFloat(void) = default;
    virtual void layout(void) const override;

public:
    std::string format;
    config::Float* value;
};

class SettingValue_SliderUnsigned final : public SettingValueWID {
public:
    virtual ~SettingValue_SliderUnsigned(void) = default;
    virtual void layout(void) const override;

public:
    config::Unsigned* value;
};

class SettingValue_StepperInt final : public SettingValue {
public:
    virtual ~SettingValue_StepperInt(void) = default;
    virtual void layout(void) const override;
    void refresh_wids(void);

public:
    std::vector<std::string> wids;
    config::Int* value;
};

class SettingValue_StepperUnsigned final : public SettingValue {
public:
    virtual ~SettingValue_StepperUnsigned(void) = default;
    virtual void layout(void) const override;
    void refresh_wids(void);

public:
    std::vector<std::string> wids;
    config::Unsigned* value;
};

class SettingValue_KeyBind final : public SettingValue {
public:
    virtual ~SettingValue_KeyBind(void) = default;
    virtual void layout(void) const override;
    void refresh_wids(void);

public:
    std::string wids[2];
    config::KeyBind* value;
};

class SettingValue_GamepadAxis final : public SettingValue {
public:
    virtual ~SettingValue_GamepadAxis(void) = default;
    virtual void layout(void) const override;
    void refresh_wids(void);

public:
    std::string wids[2];
    std::string wid_checkbox;
    config::GamepadAxis* value;
};

class SettingValue_GamepadButton final : public SettingValue {
public:
    virtual ~SettingValue_GamepadButton(void) = default;
    virtual void layout(void) const override;
    void refresh_wids(void);

public:
    std::string wids[2];
    config::GamepadButton* value;
};

class SettingValue_Language final : public SettingValueWID {
public:
    virtual ~SettingValue_Language(void) = default;
    virtual void layout(void) const override;
};

class SettingValue_VideoMode final : public SettingValueWID {
public:
    virtual ~SettingValue_VideoMode(void) override = default;
    virtual void layout(void) const override;
    void refresh_choices(void);

    std::vector<VideoMode> choices;
    std::vector<std::string> choice_labels;
};

static std::string str_label_movement;
static std::string str_label_gameplay;
static std::string str_label_miscellaneous;
static std::string str_label_gui;
static std::string str_label_levels;

static std::string str_location_general;
static std::string str_location_video;
static std::string str_location_audio;
static std::string str_location_keyboard;
static std::string str_location_mouse;
static std::string str_location_gamepad;

static std::string str_separator_settings;
static std::string str_separator_devices;

static std::string str_checkbox_false;
static std::string str_checkbox_true;

static std::string str_popup_video_change_title;
static std::string str_popup_video_change_text;
static std::string str_popup_video_change_answers[2];

static std::vector<std::shared_ptr<SettingValue>> values_all;
static std::vector<std::shared_ptr<SettingValue>> values[NUM_LOCATIONS];

static std::string str_gamepad_axis_prefix;
static std::string str_gamepad_button_prefix;
static std::string str_gamepad_checkbox_tooltip;

static bool should_open_video_mode_popup;
static const VideoMode* target_video_mode;
static gui_location selected_location;

void SettingValue::layout_tooltip(void) const
{
    if(has_tooltip) {
        ImVec2 tooltip_size(ImGui::CalcTextSize(TOOLTIP_TEXT));

        ImGui::SameLine(ImGui::GetWindowWidth() - tooltip_size.x - ImGui::GetStyle().WindowPadding.x);
        ImGui::TextDisabled(TOOLTIP_TEXT);

        if(ImGui::BeginItemTooltip()) {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 16.0f);
            ImGui::TextUnformatted(tooltip.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}

void SettingValue::layout_label(void) const
{
    ImGui::SameLine();
    ImGui::TextUnformatted(title.c_str());
}

void SettingValue_CheckBox::refresh_wids(void)
{
    wids[0] = std::format("{}###{}", str_checkbox_false, static_cast<void*>(value));
    wids[1] = std::format("{}###{}", str_checkbox_true, static_cast<void*>(value));
}

void SettingValue_CheckBox::layout(void) const
{
    const auto& wid = value->get_value() ? wids[1] : wids[0];

    if(ImGui::Button(wid.c_str(), ImVec2(ImGui::CalcItemWidth(), 0.0f))) {
        value->set_value(!value->get_value());
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_InputInt::layout(void) const
{
    auto current_value = value->get_value();

    if(ImGui::InputInt(wid.c_str(), &current_value)) {
        value->set_value(current_value);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_InputFloat::layout(void) const
{
    auto current_value = value->get_value();

    if(ImGui::InputFloat(wid.c_str(), &current_value, 0.0f, 0.0f, format.c_str())) {
        value->set_value(current_value);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_InputUnsigned::layout(void) const
{
    auto current_value = static_cast<std::uint32_t>(value->get_value());

    if(ImGui::InputScalar(wid.c_str(), ImGuiDataType_U32, &current_value)) {
        value->set_value(current_value);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_InputString::layout(void) const
{
    ImGuiInputTextFlags flags;
    std::string current_value(value->get_value());

    if(allow_whitespace) {
        flags = ImGuiInputTextFlags_AllowTabInput;
    }
    else {
        flags = 0;
    }

    if(ImGui::InputText(wid.c_str(), &current_value, flags)) {
        value->set(current_value);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_SliderInt::layout(void) const
{
    auto current_value = value->get_value();

    if(ImGui::SliderInt(wid.c_str(), &current_value, value->get_min_value(), value->get_max_value())) {
        value->set_value(current_value);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_SliderFloat::layout(void) const
{
    auto current_value = value->get_value();

    if(ImGui::SliderFloat(wid.c_str(), &current_value, value->get_min_value(), value->get_max_value(), format.c_str())) {
        value->set_value(current_value);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_SliderUnsigned::layout(void) const
{
    auto current_value = static_cast<std::uint32_t>(value->get_value());
    auto min_value = static_cast<std::uint32_t>(value->get_min_value());
    auto max_value = static_cast<std::uint32_t>(value->get_max_value());

    if(ImGui::SliderScalar(wid.c_str(), ImGuiDataType_U32, &current_value, &min_value, &max_value)) {
        value->set_value(current_value);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_StepperInt::layout(void) const
{
    auto current_value = value->get_value();
    auto min_value = value->get_min_value();
    auto max_value = value->get_max_value();

    auto current_wid = current_value - min_value;

    if(ImGui::Button(wids[current_wid].c_str(), ImVec2(ImGui::CalcItemWidth(), 0.0f))) {
        current_value += 1;
    }

    if(current_value > max_value) {
        value->set_value(min_value);
    }
    else {
        value->set_value(current_value);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_StepperInt::refresh_wids(void)
{
    for(std::size_t i = 0; i < wids.size(); ++i) {
        auto key = std::format("settings.value.{}.{}", name, i);
        wids[i] = std::format("{}###{}", language::resolve(key.c_str()), static_cast<const void*>(value));
    }
}

void SettingValue_StepperUnsigned::layout(void) const
{
    auto current_value = value->get_value();
    auto min_value = value->get_min_value();
    auto max_value = value->get_max_value();

    auto current_wid = current_value - min_value;

    if(ImGui::Button(wids[current_wid].c_str(), ImVec2(ImGui::CalcItemWidth(), 0.0f))) {
        current_value += 1U;
    }

    if(current_value > max_value) {
        value->set_value(min_value);
    }
    else {
        value->set_value(current_value);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_StepperUnsigned::refresh_wids(void)
{
    for(std::size_t i = 0; i < wids.size(); ++i) {
        auto key = std::format("settings.value.{}.{}", name, i);
        wids[i] = std::format("{}###{}", language::resolve(key.c_str()), static_cast<const void*>(value));
    }
}

void SettingValue_KeyBind::layout(void) const
{
    const auto is_active = ((globals::gui_keybind_ptr == value) && !globals::gui_gamepad_axis_ptr && !globals::gui_gamepad_button_ptr);
    const auto& wid = is_active ? wids[0] : wids[1];

    if(ImGui::Button(wid.c_str(), ImVec2(ImGui::CalcItemWidth(), 0.0f))) {
        auto& io = ImGui::GetIO();
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
        globals::gui_keybind_ptr = value;
    }

    layout_label();
}

void SettingValue_KeyBind::refresh_wids(void)
{
    wids[0] = std::format("...###{}", static_cast<const void*>(value));
    wids[1] = std::format("{}###{}", value->get(), static_cast<const void*>(value));
}

void SettingValue_GamepadAxis::layout(void) const
{
    const auto is_active = ((globals::gui_gamepad_axis_ptr == value) && !globals::gui_keybind_ptr && !globals::gui_gamepad_button_ptr);
    const auto& wid = is_active ? wids[0] : wids[1];
    auto is_inverted = value->is_inverted();

    if(ImGui::Button(wid.c_str(), ImVec2(ImGui::CalcItemWidth() - ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x, 0.0f))) {
        auto& io = ImGui::GetIO();
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
        globals::gui_gamepad_axis_ptr = value;
    }

    ImGui::SameLine();

    if(ImGui::Checkbox(wid_checkbox.c_str(), &is_inverted)) {
        value->set_inverted(is_inverted);
    }

    if(ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 16.0f);
        ImGui::TextUnformatted(str_gamepad_checkbox_tooltip.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }

    layout_label();
}

void SettingValue_GamepadAxis::refresh_wids(void)
{
    wids[0] = std::format("...###{}", static_cast<const void*>(value));
    wids[1] = std::format("{}###{}", value->get_name(), static_cast<const void*>(value));
    wid_checkbox = std::format("###CHECKBOX_{}", static_cast<const void*>(value));
}

void SettingValue_GamepadButton::layout(void) const
{
    const auto is_active = ((globals::gui_gamepad_button_ptr == value) && !globals::gui_keybind_ptr && !globals::gui_gamepad_axis_ptr);
    const auto& wid = is_active ? wids[0] : wids[1];

    if(ImGui::Button(wid.c_str(), ImVec2(ImGui::CalcItemWidth(), 0.0f))) {
        auto& io = ImGui::GetIO();
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
        globals::gui_gamepad_button_ptr = value;
    }

    layout_label();
}

void SettingValue_GamepadButton::refresh_wids(void)
{
    wids[0] = std::format("...###{}", static_cast<const void*>(value));
    wids[1] = std::format("{}###{}", value->get(), static_cast<const void*>(value));
}

void SettingValue_Language::layout(void) const
{
    auto current_language = language::get_current();

    if(ImGui::BeginCombo(wid.c_str(), current_language->endonym.c_str())) {
        for(auto it = language::cbegin(); it != language::cend(); ++it) {
            if(ImGui::Selectable(it->display.c_str(), it == current_language)) {
                language::set(it);
                continue;
            }
        }

        ImGui::EndCombo();
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_VideoMode::layout(void) const
{
    int current_width;
    int current_height;
    int current_refresh_rate = GLFW_DONT_CARE;
    bool is_fullscreen = false;

    video::query_current_mode(current_width, current_height, is_fullscreen);

    if(is_fullscreen) {
        auto monitor = glfwGetWindowMonitor(globals::window);
        auto video_mode = glfwGetVideoMode(monitor);
        current_refresh_rate = video_mode->refreshRate;
    }

    int current_index = 0;

    if(is_fullscreen) {
        for(std::size_t i = 0; i < choices.size(); ++i) {
            if(choices[i] == VideoMode(current_width, current_height, current_refresh_rate)) {
                current_index = static_cast<int>(i) + 1;
                break;
            }
        }
    }

    if(ImGui::BeginCombo(wid.c_str(), choice_labels[current_index].c_str())) {
        if(ImGui::Selectable(choice_labels[0].c_str(), current_index == 0)) {
            if(current_index != 0) {
                should_open_video_mode_popup = true;
                target_video_mode = nullptr;
            }
        }

        auto current_width = INT_MIN;
        auto current_height = INT_MIN;
        ImVec2 dummy_size(0.0f, 0.0625f * ImGui::GetStyle().ItemSpacing.y);

        for(std::size_t i = 1; i < choices.size(); ++i) {
            const auto& mode = choices[i - 1];
            auto mode_width = mode.wide();
            auto mode_height = mode.tall();

            if(current_width != mode_width || current_height != mode_height) {
                ImGui::Separator();
                ImGui::Dummy(dummy_size);

                current_height = mode_height;
                current_width = mode_width;
            }

            if(ImGui::Selectable(choice_labels[i].c_str(), current_index == static_cast<int>(i))) {
                if(current_index != static_cast<int>(i)) {
                    should_open_video_mode_popup = true;
                    target_video_mode = &mode;
                }
            }
        }

        ImGui::EndCombo();
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_VideoMode::refresh_choices(void)
{
    auto& modes = video::query_fullscreen_modes();
    unsigned int mode_index = 0U;

    choices.clear();
    choices.reserve(modes.size());

    choice_labels.clear();
    choice_labels.reserve(1 + modes.size());
    choice_labels.push_back(std::string(language::resolve("settings.choice.video_mode.windowed")));

    for(const auto& mode : modes) {
        auto mode_string = std::format("{}x{} @ {}", mode.wide(), mode.tall(), mode.rate());
        auto mode_label = std::format("{}###GUI::VideoMode[{}]", mode_string, mode_index);
        choice_labels.emplace_back(std::move(mode_label));
        choices.push_back(mode);
        mode_index += 1U;
    }
}

static void refresh_input_wids(void)
{
    for(auto& value : values_all) {
        if(value->type == setting_type::KEYBIND) {
            auto keybind = std::static_pointer_cast<SettingValue_KeyBind>(value);
            keybind->refresh_wids();
            continue;
        }

        if(value->type == setting_type::GAMEPAD_AXIS) {
            auto gamepad_axis = std::static_pointer_cast<SettingValue_GamepadAxis>(value);
            gamepad_axis->refresh_wids();
            continue;
        }

        if(value->type == setting_type::GAMEPAD_BUTTON) {
            auto gamepad_button = std::static_pointer_cast<SettingValue_GamepadButton>(value);
            gamepad_button->refresh_wids();
            continue;
        }

        if(value->type == setting_type::VIDEO_MODE_SELECT) {
            auto video_mode = std::static_pointer_cast<SettingValue_VideoMode>(value);
            video_mode->refresh_choices();
            continue;
        }
    }
}

static void on_key(const KeyEvent& event)
{
    if(event.is_action(GLFW_PRESS) && !event.is_keycode(DEBUG_KEY)) {
        if(globals::gui_keybind_ptr || globals::gui_gamepad_axis_ptr || globals::gui_gamepad_button_ptr) {
            if(event.is_keycode(GLFW_KEY_ESCAPE)) {
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

                globals::gui_keybind_ptr = nullptr;
                globals::gui_gamepad_axis_ptr = nullptr;
                globals::gui_gamepad_button_ptr = nullptr;

                return;
            }

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            globals::gui_keybind_ptr->set_key(event.keycode());
            globals::gui_keybind_ptr = nullptr;

            refresh_input_wids();

            return;
        }

        if(event.is_keycode(GLFW_KEY_ESCAPE) && globals::gui_screen == GUI_SETTINGS) {
            globals::gui_screen = GUI_MAIN_MENU;
            return;
        }
    }
}

static void on_gamepad_axis(const GamepadAxisEvent& event)
{
    if(globals::gui_gamepad_axis_ptr) {
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        globals::gui_gamepad_axis_ptr->set_axis(event.axis());
        globals::gui_gamepad_axis_ptr = nullptr;

        refresh_input_wids();

        return;
    }
}

static void on_gamepad_button(const GamepadButtonEvent& event)
{
    if(globals::gui_gamepad_button_ptr) {
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        globals::gui_gamepad_button_ptr->set_button(event.button());
        globals::gui_gamepad_button_ptr = nullptr;

        refresh_input_wids();

        return;
    }
}

static void on_language_set(const LanguageSetEvent& event)
{
    str_label_movement = language::resolve("settings.label.movement");
    str_label_gameplay = language::resolve("settings.label.gameplay");
    str_label_miscellaneous = language::resolve("settings.label.miscellaneous");
    str_label_gui = language::resolve("settings.label.gui");
    str_label_levels = language::resolve("settings.label.levels");

    str_location_general = language::resolve_gui("settings.location.general");
    str_location_keyboard = language::resolve_gui("settings.location.keyboard");
    str_location_mouse = language::resolve_gui("settings.location.mouse");
    str_location_gamepad = language::resolve_gui("settings.location.gamepad");
    str_location_video = language::resolve_gui("settings.location.video");
    str_location_audio = language::resolve_gui("settings.location.audio");

    str_separator_settings = language::resolve("settings.separator.settings");
    str_separator_devices = language::resolve("settings.separator.devices");

    str_checkbox_false = language::resolve("settings.checkbox.false");
    str_checkbox_true = language::resolve("settings.checkbox.true");

    str_popup_video_change_title = language::resolve_gui("settings.popup.video_change.title");
    str_popup_video_change_text = language::resolve("settings.popup.video_change.text");
    str_popup_video_change_answers[0] = language::resolve_gui("settings.popup.video_change.answer.yes");
    str_popup_video_change_answers[1] = language::resolve_gui("settings.popup.video_change.answer.no");

    str_gamepad_axis_prefix = language::resolve("settings.gamepad.axis");
    str_gamepad_button_prefix = language::resolve("settings.gamepad.button");
    str_gamepad_checkbox_tooltip = language::resolve("settings.gamepad.checkbox_tooltip");

    for(auto value : values_all) {
        if(value->type == setting_type::CHECKBOX) {
            auto checkbox = std::static_pointer_cast<SettingValue_CheckBox>(value);
            checkbox->refresh_wids();
        }

        if(value->type == setting_type::STEPPER_INT) {
            auto stepper = std::static_pointer_cast<SettingValue_StepperInt>(value);
            stepper->refresh_wids();
        }

        if(value->type == setting_type::STEPPER_UINT) {
            auto stepper = std::static_pointer_cast<SettingValue_StepperUnsigned>(value);
            stepper->refresh_wids();
        }

        value->title = language::resolve(std::format("settings.value.{}", value->name).c_str());

        if(value->has_tooltip) {
            value->tooltip = language::resolve(std::format("settings.tooltip.{}", value->name).c_str());
        }
    }
}

static void layout_values(settings_location location)
{
    ImGui::PushItemWidth(ImGui::CalcItemWidth() * 0.70f);

    for(auto& value : values[static_cast<unsigned int>(location)]) {
        value->layout();
    }

    ImGui::PopItemWidth();
}

static void layout_location_selectable(const std::string& label, gui_location target_location, const ImVec2& size)
{
    if(imutils::selectable_button(label.c_str(), size, selected_location == target_location)) {
        selected_location = target_location;
    }
}

void settings::init(void)
{
    selected_location = gui_location::GENERAL;
    should_open_video_mode_popup = false;
    target_video_mode = nullptr;

    globals::dispatcher.sink<KeyEvent>().connect<&on_key>();
    globals::dispatcher.sink<GamepadAxisEvent>().connect<&on_gamepad_axis>();
    globals::dispatcher.sink<GamepadButtonEvent>().connect<&on_gamepad_button>();
    globals::dispatcher.sink<LanguageSetEvent>().connect<&on_language_set>();
}

void settings::init_late(void)
{
    for(std::size_t i = 0; i < NUM_LOCATIONS; ++i) {
        std::sort(values[i].begin(), values[i].end(), [](const auto& a, const auto& b) {
            return a->priority < b->priority;
        });
    }

    refresh_input_wids();
}

void settings::shutdown(void)
{
    for(std::size_t i = 0; i < NUM_LOCATIONS; ++i) {
        values[i].clear();
    }

    values_all.clear();
}

void settings::layout(void)
{
    const auto viewport = ImGui::GetMainViewport();
    auto window_start = ImVec2(viewport->Size.x * 0.025f, viewport->Size.y * 0.025f);
    auto window_size = ImVec2(viewport->Size.x * 0.95f, viewport->Size.y * 0.95f);

    ImGui::SetNextWindowPos(window_start);
    ImGui::SetNextWindowSize(window_size);

    ImGui::PushFont(globals::font_unscii16, 16.0f);
    ImGui::PushStyleVarX(ImGuiStyleVar_FramePadding, 8.0f * globals::gui_scale);
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 3.2f * globals::gui_scale);

    if(ImGui::Begin("###settings.window", nullptr, WINDOW_FLAGS)) {
        auto sidebar_width = 0.25f * window_size.x;
        auto draw_list = ImGui::GetWindowDrawList();
        auto separator_color = ImGui::GetColorU32(ImGuiCol_Text);

        if(ImGui::BeginChild("###settings.window.sidebar", ImVec2(sidebar_width, -1.0f), 0, ImGuiWindowFlags_AlwaysUseWindowPadding)) {
            auto selectable_height = 24.0f * globals::gui_scale;
            auto selectable_size = ImVec2(ImGui::GetContentRegionAvail().x, selectable_height);

            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

            ImGui::BeginDisabled(globals::gui_gamepad_axis_ptr || globals::gui_gamepad_button_ptr || globals::gui_keybind_ptr);

            if(ImGui::Button("<<###settings.window.back", ImVec2(-1.0f, 0.0f))) {
                globals::gui_screen = GUI_MAIN_MENU;

                if(globals::gui_keybind_ptr) {
                    globals::gui_keybind_ptr->set_key(GLFW_KEY_UNKNOWN);
                    globals::gui_keybind_ptr = nullptr;
                }
            }

            ImGui::SeparatorText(str_separator_settings.c_str());
            layout_location_selectable(str_location_general, gui_location::GENERAL, selectable_size);
            layout_location_selectable(str_location_video, gui_location::VIDEO, selectable_size);
            layout_location_selectable(str_location_audio, gui_location::AUDIO, selectable_size);

            ImGui::SeparatorText(str_separator_devices.c_str());
            layout_location_selectable(str_location_keyboard, gui_location::KEYBOARD, selectable_size);
            layout_location_selectable(str_location_mouse, gui_location::MOUSE, selectable_size);
            layout_location_selectable(str_location_gamepad, gui_location::GAMEPAD, selectable_size);

            ImGui::EndDisabled();

            ImGui::PopStyleVar();
        }

        ImGui::EndChild();
        ImGui::SameLine();

        if(ImGui::BeginChild("###settings.window.contents", ImVec2(-1.0f, -1.0f), 0, ImGuiWindowFlags_AlwaysUseWindowPadding)) {
            switch(selected_location) {
                case gui_location::GENERAL:
                    layout_values(settings_location::GENERAL);
                    break;

                case gui_location::VIDEO:
                    layout_values(settings_location::VIDEO);
                    ImGui::SeparatorText(str_label_gui.c_str());
                    layout_values(settings_location::VIDEO_GUI);
                    break;

                case gui_location::AUDIO:
                    ImGui::BeginDisabled(globals::sound_ctx == nullptr);
                    layout_values(settings_location::SOUND);
                    ImGui::SeparatorText(str_label_levels.c_str());
                    layout_values(settings_location::SOUND_LEVELS);
                    ImGui::EndDisabled();
                    break;

                case gui_location::KEYBOARD:
                    ImGui::SeparatorText(str_label_movement.c_str());
                    layout_values(settings_location::KEYBOARD_MOVEMENT);
                    ImGui::SeparatorText(str_label_gameplay.c_str());
                    layout_values(settings_location::KEYBOARD_GAMEPLAY);
                    ImGui::SeparatorText(str_label_miscellaneous.c_str());
                    layout_values(settings_location::KEYBOARD_MISC);
                    break;

                case gui_location::MOUSE:
                    layout_values(settings_location::MOUSE);
                    break;

                case gui_location::GAMEPAD:
                    ImGui::SeparatorText(str_label_movement.c_str());
                    layout_values(settings_location::GAMEPAD_MOVEMENT);
                    ImGui::SeparatorText(str_label_gameplay.c_str());
                    layout_values(settings_location::GAMEPAD_GAMEPLAY);
                    ImGui::SeparatorText(str_label_miscellaneous.c_str());
                    layout_values(settings_location::GAMEPAD_MISC);
                    break;
            }
        }

        ImGui::EndChild();
    }

    if(should_open_video_mode_popup) {
        ImGui::OpenPopup(str_popup_video_change_title.c_str());

        should_open_video_mode_popup = false;
    }

    ImGui::PopStyleVar(2);
    ImGui::PopFont();

    if(0 == imutils::popup(str_popup_video_change_title, str_popup_video_change_text, str_popup_video_change_answers, 2, 1.0f)) {
        if(target_video_mode) {
            video::request_fullscreen(target_video_mode->wide(), target_video_mode->tall(), target_video_mode->rate());
        }
        else {
            video::request_windowed();
        }
    }

    ImGui::End();
}

void settings::add_checkbox(int priority, config::Boolean& value, settings_location location, std::string_view name, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_CheckBox>();
    setting_value->type = setting_type::CHECKBOX;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_input(int priority, config::Int& value, settings_location location, std::string_view name, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_InputInt>();
    setting_value->type = setting_type::INPUT_INT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_input(int priority, config::Float& value, settings_location location, std::string_view name, bool tooltip,
    std::string_view fmt)
{
    auto setting_value = std::make_shared<SettingValue_InputFloat>();
    setting_value->type = setting_type::INPUT_FLOAT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->format = fmt;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_input(int priority, config::Unsigned& value, settings_location location, std::string_view name, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_InputUnsigned>();
    setting_value->type = setting_type::INPUT_UINT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_input(int priority, config::String& value, settings_location location, std::string_view name, bool tooltip,
    bool allow_whitespace)
{
    auto setting_value = std::make_shared<SettingValue_InputString>();
    setting_value->type = setting_type::INPUT_STRING;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->allow_whitespace = allow_whitespace;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_slider(int priority, config::Int& value, settings_location location, std::string_view name, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_SliderInt>();
    setting_value->type = setting_type::SLIDER_INT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_slider(int priority, config::Float& value, settings_location location, std::string_view name, bool tooltip,
    std::string_view fmt)
{
    auto setting_value = std::make_shared<SettingValue_SliderFloat>();
    setting_value->type = setting_type::SLIDER_FLOAT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->format = fmt;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_slider(int priority, config::Unsigned& value, settings_location location, std::string_view name, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_SliderUnsigned>();
    setting_value->type = setting_type::SLIDER_UINT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_stepper(int priority, config::Int& value, settings_location location, std::string_view name, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_StepperInt>();
    setting_value->type = setting_type::STEPPER_INT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wids.resize(value.get_max_value() - value.get_min_value() + 1);
    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_stepper(int priority, config::Unsigned& value, settings_location location, std::string_view name, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_StepperUnsigned>();
    setting_value->type = setting_type::STEPPER_UINT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wids.resize(value.get_max_value() - value.get_min_value() + 1);
    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_keybind(int priority, config::KeyBind& value, settings_location location, std::string_view name)
{
    auto setting_value = std::make_shared<SettingValue_KeyBind>();
    setting_value->type = setting_type::KEYBIND;
    setting_value->priority = priority;
    setting_value->has_tooltip = false;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_gamepad_axis(int priority, config::GamepadAxis& value, settings_location location, std::string_view name)
{
    auto setting_value = std::make_shared<SettingValue_GamepadAxis>();
    setting_value->type = setting_type::GAMEPAD_AXIS;
    setting_value->priority = priority;
    setting_value->has_tooltip = false;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_gamepad_button(int priority, config::GamepadButton& value, settings_location location, std::string_view name)
{
    auto setting_value = std::make_shared<SettingValue_GamepadButton>();
    setting_value->type = setting_type::GAMEPAD_BUTTON;
    setting_value->priority = priority;
    setting_value->has_tooltip = false;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_language_select(int priority, settings_location location, std::string_view name)
{
    auto setting_value = std::make_shared<SettingValue_Language>();
    setting_value->type = setting_type::LANGUAGE_SELECT;
    setting_value->priority = priority;
    setting_value->has_tooltip = false;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_video_mode_select(int priority, settings_location location, std::string_view name)
{
    auto setting_value = std::make_shared<SettingValue_VideoMode>();
    setting_value->type = setting_type::VIDEO_MODE_SELECT;
    setting_value->priority = priority;
    setting_value->has_tooltip = false;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));
    setting_value->refresh_choices();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}
