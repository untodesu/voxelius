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
#include "client/gui/language.hh"
#include "client/io/gamepad.hh"
#include "client/io/glfw.hh"

#include "client/const.hh"
#include "client/globals.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;
constexpr static unsigned int NUM_LOCATIONS = static_cast<unsigned int>(settings_location::COUNT);

enum class setting_type : unsigned int {
    CHECKBOX = 0x0000U,        ///< config::Boolean
    INPUT_INT = 0x0001U,       ///< config::Number<int>
    INPUT_FLOAT = 0x0002U,     ///< config::Number<float>
    INPUT_UINT = 0x0003U,      ///< config::Number<unsigned int>
    INPUT_STRING = 0x0004U,    ///< config::String
    SLIDER_INT = 0x0005U,      ///< config::Number<int>
    SLIDER_FLOAT = 0x0006U,    ///< config::Number<float>
    SLIDER_UINT = 0x0007U,     ///< config::Number<unsigned int>
    STEPPER_INT = 0x0008U,     ///< config::Number<int>
    STEPPER_UINT = 0x0009U,    ///< config::Number<unsigned int>
    KEYBIND = 0x000AU,         ///< config::KeyBind
    GAMEPAD_AXIS = 0x000BU,    ///< config::GamepadAxis
    GAMEPAD_BUTTON = 0x000CU,  ///< config::GamepadButton
    LANGUAGE_SELECT = 0x000DU, ///< config::String internally
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

static std::string str_checkbox_false;
static std::string str_checkbox_true;

static std::string str_tab_general;
static std::string str_tab_input;
static std::string str_tab_video;
static std::string str_tab_sound;

static std::string str_input_keyboard;
static std::string str_input_gamepad;
static std::string str_input_mouse;

static std::string str_keyboard_movement;
static std::string str_keyboard_gameplay;
static std::string str_keyboard_misc;

static std::string str_gamepad_movement;
static std::string str_gamepad_gameplay;
static std::string str_gamepad_misc;

static std::string str_gamepad_axis_prefix;
static std::string str_gamepad_button_prefix;
static std::string str_gamepad_checkbox_tooltip;

static std::string str_video_gui;

static std::string str_sound_levels;

static std::vector<SettingValue*> values_all;
static std::vector<SettingValue*> values[NUM_LOCATIONS];

void SettingValue::layout_tooltip(void) const
{
    if(has_tooltip) {
        ImGui::SameLine();
        ImGui::TextDisabled("[?]");

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
    std::string current_value = value->get();

    if(allow_whitespace) {
        flags = ImGuiInputTextFlags_AllowTabInput;
    }
    else {
        flags = 0;
    }

    if(ImGui::InputText(wid.c_str(), &current_value, flags)) {
        value->set(current_value.c_str());
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
        wids[i] = std::format("{}###{}", gui::language::resolve(key.c_str()), static_cast<const void*>(value));
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
        wids[i] = std::format("{}###{}", gui::language::resolve(key.c_str()), static_cast<const void*>(value));
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
    auto current_language = gui::language::get_current();

    if(ImGui::BeginCombo(wid.c_str(), current_language->endonym.c_str())) {
        for(auto it = gui::language::cbegin(); it != gui::language::cend(); ++it) {
            if(ImGui::Selectable(it->display.c_str(), it == current_language)) {
                gui::language::set(it);
                continue;
            }
        }

        ImGui::EndCombo();
    }

    layout_label();
    layout_tooltip();
}

static void refresh_input_wids(void)
{
    for(SettingValue* value : values_all) {
        if(value->type == setting_type::KEYBIND) {
            auto keybind = static_cast<SettingValue_KeyBind*>(value);
            keybind->refresh_wids();
            continue;
        }

        if(value->type == setting_type::GAMEPAD_AXIS) {
            auto gamepad_axis = static_cast<SettingValue_GamepadAxis*>(value);
            gamepad_axis->refresh_wids();
            continue;
        }

        if(value->type == setting_type::GAMEPAD_BUTTON) {
            auto gamepad_button = static_cast<SettingValue_GamepadButton*>(value);
            gamepad_button->refresh_wids();
        }
    }
}

static void on_glfw_key(const io::GlfwKeyEvent& event)
{
    if((event.action == GLFW_PRESS) && (event.key != DEBUG_KEY)) {
        if(globals::gui_keybind_ptr || globals::gui_gamepad_axis_ptr || globals::gui_gamepad_button_ptr) {
            if(event.key == GLFW_KEY_ESCAPE) {
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

                globals::gui_keybind_ptr = nullptr;
                globals::gui_gamepad_axis_ptr = nullptr;
                globals::gui_gamepad_button_ptr = nullptr;

                return;
            }

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            globals::gui_keybind_ptr->set_key(event.key);
            globals::gui_keybind_ptr = nullptr;

            refresh_input_wids();

            return;
        }

        if((event.key == GLFW_KEY_ESCAPE) && (globals::gui_screen == GUI_SETTINGS)) {
            globals::gui_screen = GUI_MAIN_MENU;
            return;
        }
    }
}

static void on_gamepad_axis(const io::GamepadAxisEvent& event)
{
    if(globals::gui_gamepad_axis_ptr) {
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        globals::gui_gamepad_axis_ptr->set_axis(event.axis);
        globals::gui_gamepad_axis_ptr = nullptr;

        refresh_input_wids();

        return;
    }
}

static void on_gamepad_button(const io::GamepadButtonEvent& event)
{
    if(globals::gui_gamepad_button_ptr) {
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        globals::gui_gamepad_button_ptr->set_button(event.button);
        globals::gui_gamepad_button_ptr = nullptr;

        refresh_input_wids();

        return;
    }
}

static void on_language_set(const gui::LanguageSetEvent& event)
{
    str_checkbox_false = gui::language::resolve("settings.checkbox.false");
    str_checkbox_true = gui::language::resolve("settings.checkbox.true");

    str_tab_general = gui::language::resolve("settings.tab.general");
    str_tab_input = gui::language::resolve("settings.tab.input");
    str_tab_video = gui::language::resolve("settings.tab.video");
    str_tab_sound = gui::language::resolve("settings.tab.sound");

    str_input_keyboard = gui::language::resolve("settings.input.keyboard");
    str_input_gamepad = gui::language::resolve("settings.input.gamepad");
    str_input_mouse = gui::language::resolve("settings.input.mouse");

    str_keyboard_movement = gui::language::resolve("settings.keyboard.movement");
    str_keyboard_gameplay = gui::language::resolve("settings.keyboard.gameplay");
    str_keyboard_misc = gui::language::resolve("settings.keyboard.misc");

    str_gamepad_movement = gui::language::resolve("settings.gamepad.movement");
    str_gamepad_gameplay = gui::language::resolve("settings.gamepad.gameplay");
    str_gamepad_misc = gui::language::resolve("settings.gamepad.misc");

    str_gamepad_axis_prefix = gui::language::resolve("settings.gamepad.axis");
    str_gamepad_button_prefix = gui::language::resolve("settings.gamepad.button");
    str_gamepad_checkbox_tooltip = gui::language::resolve("settings.gamepad.checkbox_tooltip");

    str_video_gui = gui::language::resolve("settings.video.gui");

    str_sound_levels = gui::language::resolve("settings.sound.levels");

    for(SettingValue* value : values_all) {
        if(value->type == setting_type::CHECKBOX) {
            auto checkbox = static_cast<SettingValue_CheckBox*>(value);
            checkbox->refresh_wids();
        }

        if(value->type == setting_type::STEPPER_INT) {
            auto stepper = static_cast<SettingValue_StepperInt*>(value);
            stepper->refresh_wids();
        }

        if(value->type == setting_type::STEPPER_UINT) {
            auto stepper = static_cast<SettingValue_StepperUnsigned*>(value);
            stepper->refresh_wids();
        }

        value->title = gui::language::resolve(std::format("settings.value.{}", value->name).c_str());

        if(value->has_tooltip) {
            value->tooltip = gui::language::resolve(std::format("settings.tooltip.{}", value->name).c_str());
        }
    }
}

static void layout_values(settings_location location)
{
    ImGui::PushItemWidth(ImGui::CalcItemWidth() * 0.70f);

    for(const SettingValue* value : values[static_cast<unsigned int>(location)]) {
        value->layout();
    }

    ImGui::PopItemWidth();
}

static void layout_general(void)
{
    if(ImGui::BeginChild("###settings.general.child")) {
        layout_values(settings_location::GENERAL);
    }

    ImGui::EndChild();
}

static void layout_input_keyboard(void)
{
    if(ImGui::BeginChild("###settings.input.keyboard.child")) {
        ImGui::SeparatorText(str_keyboard_movement.c_str());
        layout_values(settings_location::KEYBOARD_MOVEMENT);
        ImGui::SeparatorText(str_keyboard_gameplay.c_str());
        layout_values(settings_location::KEYBOARD_GAMEPLAY);
        ImGui::SeparatorText(str_keyboard_misc.c_str());
        layout_values(settings_location::KEYBOARD_MISC);
    }

    ImGui::EndChild();
}

static void layout_input_gamepad(void)
{
    if(ImGui::BeginChild("###settings.input.gamepad.child")) {
        layout_values(settings_location::GAMEPAD);
        ImGui::SeparatorText(str_gamepad_movement.c_str());
        layout_values(settings_location::GAMEPAD_MOVEMENT);
        ImGui::SeparatorText(str_gamepad_gameplay.c_str());
        layout_values(settings_location::GAMEPAD_GAMEPLAY);
        ImGui::SeparatorText(str_gamepad_misc.c_str());
        layout_values(settings_location::GAMEPAD_MISC);
    }

    ImGui::EndChild();
}

static void layout_input_mouse(void)
{
    if(ImGui::BeginChild("###settings.input.mouse.child")) {
        layout_values(settings_location::MOUSE);
    }

    ImGui::EndChild();
}

static void layout_input(void)
{
    if(ImGui::BeginTabBar("###settings.input.tabs", ImGuiTabBarFlags_FittingPolicyResizeDown)) {
        if(ImGui::BeginTabItem(str_input_keyboard.c_str())) {
            layout_input_keyboard();
            ImGui::EndTabItem();
        }

        if(io::gamepad::available) {
            if(ImGui::BeginTabItem(str_input_gamepad.c_str())) {
                globals::gui_keybind_ptr = nullptr;
                layout_input_gamepad();
                ImGui::EndTabItem();
            }
        }

        if(ImGui::BeginTabItem(str_input_mouse.c_str())) {
            globals::gui_keybind_ptr = nullptr;
            layout_input_mouse();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

static void layout_video(void)
{
    if(ImGui::BeginChild("###settings.video.child")) {
        layout_values(settings_location::VIDEO);
        ImGui::SeparatorText(str_video_gui.c_str());
        layout_values(settings_location::VIDEO_GUI);
    }

    ImGui::EndChild();
}

static void layout_sound(void)
{
    if(ImGui::BeginChild("###settings.sound.child")) {
        layout_values(settings_location::SOUND);
        ImGui::SeparatorText(str_sound_levels.c_str());
        layout_values(settings_location::SOUND_LEVELS);
    }

    ImGui::EndChild();
}

void settings::init(void)
{
    globals::dispatcher.sink<io::GlfwKeyEvent>().connect<&on_glfw_key>();
    globals::dispatcher.sink<io::GamepadAxisEvent>().connect<&on_gamepad_axis>();
    globals::dispatcher.sink<io::GamepadButtonEvent>().connect<&on_gamepad_button>();
    globals::dispatcher.sink<gui::LanguageSetEvent>().connect<&on_language_set>();
}

void settings::init_late(void)
{
    for(std::size_t i = 0; i < NUM_LOCATIONS; ++i) {
        std::sort(values[i].begin(), values[i].end(), [](const SettingValue* a, const SettingValue* b) {
            return a->priority < b->priority;
        });
    }

    refresh_input_wids();
}

void settings::shutdown(void)
{
    for(const SettingValue* value : values_all)
        delete value;
    for(std::size_t i = 0; i < NUM_LOCATIONS; values[i++].clear())
        ;
    values_all.clear();
}

void settings::layout(void)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 window_start = ImVec2(viewport->Size.x * 0.05f, viewport->Size.y * 0.05f);
    const ImVec2 window_size = ImVec2(viewport->Size.x * 0.90f, viewport->Size.y * 0.90f);

    ImGui::SetNextWindowPos(window_start);
    ImGui::SetNextWindowSize(window_size);

    if(ImGui::Begin("###settings", nullptr, WINDOW_FLAGS)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3.0f * globals::gui_scale, 3.0f * globals::gui_scale));

        if(ImGui::BeginTabBar("###settings.tabs", ImGuiTabBarFlags_FittingPolicyResizeDown)) {
            if(ImGui::TabItemButton("<<")) {
                globals::gui_screen = GUI_MAIN_MENU;
                globals::gui_keybind_ptr = nullptr;
            }

            if(ImGui::BeginTabItem(str_tab_general.c_str())) {
                globals::gui_keybind_ptr = nullptr;
                layout_general();
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem(str_tab_input.c_str())) {
                layout_input();
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem(str_tab_video.c_str())) {
                globals::gui_keybind_ptr = nullptr;
                layout_video();
                ImGui::EndTabItem();
            }

            if(globals::sound_ctx && globals::sound_dev) {
                if(ImGui::BeginTabItem(str_tab_sound.c_str())) {
                    globals::gui_keybind_ptr = nullptr;
                    layout_sound();
                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }

        ImGui::PopStyleVar();
    }

    ImGui::End();
}

void settings::add_checkbox(int priority, config::Boolean& value, settings_location location, const char* name, bool tooltip)
{
    auto setting_value = new SettingValue_CheckBox;
    setting_value->type = setting_type::CHECKBOX;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_input(int priority, config::Int& value, settings_location location, const char* name, bool tooltip)
{
    auto setting_value = new SettingValue_InputInt;
    setting_value->type = setting_type::INPUT_INT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_input(int priority, config::Float& value, settings_location location, const char* name, bool tooltip, const char* format)
{
    auto setting_value = new SettingValue_InputFloat;
    setting_value->type = setting_type::INPUT_FLOAT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_input(int priority, config::Unsigned& value, settings_location location, const char* name, bool tooltip)
{
    auto setting_value = new SettingValue_InputUnsigned;
    setting_value->type = setting_type::INPUT_UINT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_input(
    int priority, config::String& value, settings_location location, const char* name, bool tooltip, bool allow_whitespace)
{
    auto setting_value = new SettingValue_InputString;
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

void settings::add_slider(int priority, config::Int& value, settings_location location, const char* name, bool tooltip)
{
    auto setting_value = new SettingValue_SliderInt;
    setting_value->type = setting_type::SLIDER_INT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_slider(
    int priority, config::Float& value, settings_location location, const char* name, bool tooltip, const char* format)
{
    auto setting_value = new SettingValue_SliderFloat;
    setting_value->type = setting_type::SLIDER_FLOAT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->format = format;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_slider(int priority, config::Unsigned& value, settings_location location, const char* name, bool tooltip)
{
    auto setting_value = new SettingValue_SliderUnsigned;
    setting_value->type = setting_type::SLIDER_UINT;
    setting_value->priority = priority;
    setting_value->has_tooltip = tooltip;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value->value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_stepper(int priority, config::Int& value, settings_location location, const char* name, bool tooltip)
{
    auto setting_value = new SettingValue_StepperInt;
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

void settings::add_stepper(int priority, config::Unsigned& value, settings_location location, const char* name, bool tooltip)
{
    auto setting_value = new SettingValue_StepperUnsigned;
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

void settings::add_keybind(int priority, config::KeyBind& value, settings_location location, const char* name)
{
    auto setting_value = new SettingValue_KeyBind;
    setting_value->type = setting_type::KEYBIND;
    setting_value->priority = priority;
    setting_value->has_tooltip = false;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_gamepad_axis(int priority, config::GamepadAxis& value, settings_location location, const char* name)
{
    auto setting_value = new SettingValue_GamepadAxis;
    setting_value->type = setting_type::GAMEPAD_AXIS;
    setting_value->priority = priority;
    setting_value->has_tooltip = false;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_gamepad_button(int priority, config::GamepadButton& value, settings_location location, const char* name)
{
    auto setting_value = new SettingValue_GamepadButton;
    setting_value->type = setting_type::GAMEPAD_BUTTON;
    setting_value->priority = priority;
    setting_value->has_tooltip = false;
    setting_value->value = &value;
    setting_value->name = name;

    setting_value->refresh_wids();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::add_language_select(int priority, settings_location location, const char* name)
{
    auto setting_value = new SettingValue_Language;
    setting_value->type = setting_type::LANGUAGE_SELECT;
    setting_value->priority = priority;
    setting_value->has_tooltip = false;
    setting_value->name = name;

    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value));

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}
