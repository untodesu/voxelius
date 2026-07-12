#include "client/pch.hh"

#include "client/settings.hh"

#include "core/config/map.hh"
#include "core/config/ref.hh"

#include "client/fonts.hh"
#include "client/globals.hh"
#include "client/gui.hh"
#include "client/language.hh"
#include "client/utils/imgui.hh"
#include "client/video.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration;
constexpr static unsigned int NUM_LOCATIONS = static_cast<unsigned int>(settings_location::count);
constexpr static std::string_view TOOLTIP_TEXT = "[?]";

enum class gui_location {
    GENERAL,
    VIDEO,
    AUDIO,
    KEYBOARD,
    MOUSE,
    GAMEPAD,
};

class SettingValue {
public:
    virtual ~SettingValue(void) = default;
    virtual void layout(void) const = 0;
    virtual void refresh_locale(void);
    void layout_tooltip(void) const;
    void layout_label(void) const;

public:
    std::string title;
    std::string tooltip;
    std::string key;
    settings_location location {};
    int priority { 0 };
    bool has_tooltip { false };
};

class SettingValueWID : public SettingValue {
public:
    virtual ~SettingValueWID(void) override = default;

public:
    std::string wid;
};

class SettingValue_CheckBox final : public SettingValue {
public:
    virtual ~SettingValue_CheckBox(void) override = default;
    virtual void layout(void) const override;
    virtual void refresh_locale(void) override;

public:
    mutable config::Ref<bool> value;
    std::string wids[2];
};

class SettingValue_InputInt final : public SettingValueWID {
public:
    virtual ~SettingValue_InputInt(void) override = default;
    virtual void layout(void) const override;

public:
    mutable config::Ref<int> value;
    int min_value;
    int max_value;
};

class SettingValue_InputFloat final : public SettingValueWID {
public:
    virtual ~SettingValue_InputFloat(void) override = default;
    virtual void layout(void) const override;

public:
    mutable config::Ref<float> value;
    std::string format;
    float min_value;
    float max_value;
};

class SettingValue_InputUnsigned final : public SettingValueWID {
public:
    virtual ~SettingValue_InputUnsigned(void) override = default;
    virtual void layout(void) const override;

public:
    mutable config::Ref<unsigned> value;
    unsigned min_value;
    unsigned max_value;
};

class SettingValue_InputString final : public SettingValueWID {
public:
    virtual ~SettingValue_InputString(void) override = default;
    virtual void layout(void) const override;

public:
    mutable config::Ref<std::string> value;
    bool allow_whitespace;
};

class SettingValue_SliderInt final : public SettingValueWID {
public:
    virtual ~SettingValue_SliderInt(void) override = default;
    virtual void layout(void) const override;

public:
    mutable config::Ref<int> value;
    int min_value;
    int max_value;
};

class SettingValue_SliderFloat final : public SettingValueWID {
public:
    virtual ~SettingValue_SliderFloat(void) override = default;
    virtual void layout(void) const override;

public:
    mutable config::Ref<float> value;
    std::string format;
    float min_value;
    float max_value;
};

class SettingValue_SliderUnsigned final : public SettingValueWID {
public:
    virtual ~SettingValue_SliderUnsigned(void) override = default;
    virtual void layout(void) const override;

public:
    mutable config::Ref<unsigned> value;
    unsigned min_value;
    unsigned max_value;
};

class SettingValue_StepperInt final : public SettingValue {
public:
    virtual ~SettingValue_StepperInt(void) override = default;
    virtual void layout(void) const override;
    virtual void refresh_locale(void) override;

public:
    mutable config::Ref<int> value;
    int min_value;
    int max_value;
    int step_value;
    std::vector<std::string> wids;
};

class SettingValue_StepperUnsigned final : public SettingValue {
public:
    virtual ~SettingValue_StepperUnsigned(void) override = default;
    virtual void layout(void) const override;
    virtual void refresh_locale(void) override;

public:
    mutable config::Ref<unsigned> value;
    unsigned min_value;
    unsigned max_value;
    unsigned step_value;
    std::vector<std::string> wids;
};

class SettingValue_Language final : public SettingValueWID {
public:
    virtual ~SettingValue_Language(void) override = default;
    virtual void layout(void) const override;
};

class SettingValue_VideoMode final : public SettingValueWID {
public:
    virtual ~SettingValue_VideoMode(void) override = default;
    virtual void layout(void) const override;
    virtual void refresh_locale(void) override;
    void refresh_choices(void);

public:
    std::vector<SDL_DisplayMode> choices;
    std::vector<std::string> choice_labels;
};

static std::string str_location_general;
static std::string str_location_video;
static std::string str_location_audio;
static std::string str_location_keyboard;
static std::string str_location_mouse;
static std::string str_location_gamepad;

static std::string str_separator_settings;
static std::string str_separator_devices;

static std::string str_label_gui;
static std::string str_label_levels;
static std::string str_label_movement;
static std::string str_label_gameplay;
static std::string str_label_miscellaneous;

static std::string str_checkbox_false;
static std::string str_checkbox_true;

static std::string str_popup_video_change_title;
static std::string str_popup_video_change_text;
static std::string str_popup_video_change_choices[2];

static std::vector<std::shared_ptr<SettingValue>> values_all;
static std::vector<std::shared_ptr<SettingValue>> values[NUM_LOCATIONS];

static gui_location selected_location;

static bool should_open_video_mode_popup;
static bool target_fullscreen;
static int target_width;
static int target_height;
static int target_rate;

void SettingValue::refresh_locale(void)
{
    title = std::string(language::resolve(std::format("settings.value.{}", key)));

    if(has_tooltip) {
        tooltip = std::string(language::resolve(std::format("settings.tooltip.{}", key)));
    }
}

void SettingValue::layout_tooltip(void) const
{
    if(has_tooltip) {
        ImVec2 tooltip_size(ImGui::CalcTextSize(TOOLTIP_TEXT.data()));

        ImGui::SameLine(ImGui::GetWindowWidth() - tooltip_size.x - ImGui::GetStyle().WindowPadding.x);
        ImGui::TextDisabled("%s", TOOLTIP_TEXT.data());

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

void SettingValue_CheckBox::refresh_locale(void)
{
    SettingValue::refresh_locale();

    wids[0] = std::format("{}###{}", str_checkbox_false, static_cast<const void*>(this));
    wids[1] = std::format("{}###{}", str_checkbox_true, static_cast<const void*>(this));
}

void SettingValue_CheckBox::layout(void) const
{
    auto current = value.value();
    const auto& wid = current ? wids[1] : wids[0];

    if(utils::toggle_button(wid.c_str(), ImVec2(ImGui::CalcItemWidth(), 0.0f), current)) {
        value.set_value(current);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_InputInt::layout(void) const
{
    auto current = value.value();

    if(ImGui::InputInt(wid.c_str(), &current)) {
        value.set_value(std::clamp(current, min_value, max_value));
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_InputFloat::layout(void) const
{
    auto current = value.value();

    if(ImGui::InputFloat(wid.c_str(), &current, 0.0f, 0.0f, format.c_str())) {
        value.set_value(std::clamp(current, min_value, max_value));
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_InputUnsigned::layout(void) const
{
    auto current = static_cast<std::uint32_t>(value.value());

    if(ImGui::InputScalar(wid.c_str(), ImGuiDataType_U32, &current)) {
        value.set_value(std::clamp(static_cast<unsigned>(current), min_value, max_value));
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_InputString::layout(void) const
{
    std::string current = value.value();
    ImGuiInputTextFlags flags = allow_whitespace ? ImGuiInputTextFlags_AllowTabInput : 0;

    if(ImGui::InputText(wid.c_str(), &current, flags)) {
        value.set_value(current);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_SliderInt::layout(void) const
{
    auto current = value.value();

    if(ImGui::SliderInt(wid.c_str(), &current, min_value, max_value)) {
        value.set_value(current);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_SliderFloat::layout(void) const
{
    auto current = value.value();

    if(ImGui::SliderFloat(wid.c_str(), &current, min_value, max_value, format.c_str())) {
        value.set_value(current);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_SliderUnsigned::layout(void) const
{
    auto current = static_cast<std::uint32_t>(value.value());
    auto slider_min = static_cast<std::uint32_t>(min_value);
    auto slider_max = static_cast<std::uint32_t>(max_value);

    if(ImGui::SliderScalar(wid.c_str(), ImGuiDataType_U32, &current, &slider_min, &slider_max)) {
        value.set_value(static_cast<unsigned>(current));
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_StepperInt::refresh_locale(void)
{
    SettingValue::refresh_locale();

    for(std::size_t i = 0; i < wids.size(); ++i) {
        auto step_key = std::format("settings.value.{}.{}", key, i);
        wids[i] = std::format("{}###{}", language::resolve(step_key), static_cast<const void*>(this));
    }
}

void SettingValue_StepperInt::layout(void) const
{
    auto current = value.value();
    auto index = (current - min_value) / step_value;

    if(ImGui::Button(wids[index].c_str(), ImVec2(ImGui::CalcItemWidth(), 0.0f))) {
        current += step_value;

        if(current > max_value) {
            current = min_value;
        }

        value.set_value(current);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_StepperUnsigned::refresh_locale(void)
{
    SettingValue::refresh_locale();

    for(std::size_t i = 0; i < wids.size(); ++i) {
        auto step_key = std::format("settings.value.{}.{}", key, i);

        wids[i] = std::format("{}###{}", language::resolve(step_key), static_cast<const void*>(this));
    }
}

void SettingValue_StepperUnsigned::layout(void) const
{
    auto current = value.value();
    auto index = (current - min_value) / step_value;

    if(ImGui::Button(wids[index].c_str(), ImVec2(ImGui::CalcItemWidth(), 0.0f))) {
        current += step_value;

        if(current > max_value) {
            current = min_value;
        }

        value.set_value(current);
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_Language::layout(void) const
{
    auto current = language::current();
    std::string current_label(current->endonym());

    if(ImGui::BeginCombo(wid.c_str(), current_label.c_str())) {
        for(auto it = language::const_begin(); it != language::const_end(); it = std::next(it)) {
            std::string choice_label(it->display());

            if(ImGui::Selectable(choice_label.c_str(), it == current)) {
                language::set(it);
            }
        }

        ImGui::EndCombo();
    }

    layout_label();
    layout_tooltip();
}

void SettingValue_VideoMode::refresh_locale(void)
{
    SettingValue::refresh_locale();

    choice_labels[0] = std::string(language::resolve("settings.choice.video_mode.windowed"));
}

void SettingValue_VideoMode::refresh_choices(void)
{
    choices = video::query_fullscreen_modes();

    choice_labels.clear();
    choice_labels.resize(1 + choices.size());

    for(std::size_t i = 0; i < choices.size(); ++i) {
        const auto& mode = choices[i];
        choice_labels[i + 1] = std::format("{}x{} @ {}###gui::video_mode[{}]", mode.w, mode.h, static_cast<int>(mode.refresh_rate), i);
    }
}

void SettingValue_VideoMode::layout(void) const
{
    int current_width;
    int current_height;
    int current_rate;
    bool is_fullscreen;

    video::query_current_mode(current_width, current_height, current_rate, is_fullscreen);

    int current_index = 0;

    if(is_fullscreen) {
        for(std::size_t i = 0; i < choices.size(); ++i) {
            const auto& mode = choices[i];

            if(mode.w == current_width && mode.h == current_height && static_cast<int>(mode.refresh_rate) == current_rate) {
                current_index = static_cast<int>(i) + 1;
                break;
            }
        }
    }

    if(ImGui::BeginCombo(wid.c_str(), choice_labels[current_index].c_str())) {
        if(ImGui::Selectable(choice_labels[0].c_str(), current_index == 0)) {
            if(current_index != 0) {
                should_open_video_mode_popup = true;
                target_fullscreen = false;
            }
        }

        auto group_width = INT_MIN;
        auto group_height = INT_MIN;
        ImVec2 group_gap(0.0f, 0.0625f * ImGui::GetStyle().ItemSpacing.y);

        for(std::size_t i = 0; i < choices.size(); ++i) {
            const auto& mode = choices[i];
            auto choice_index = static_cast<int>(i) + 1;

            if(group_width != mode.w || group_height != mode.h) {
                ImGui::Separator();
                ImGui::Dummy(group_gap);

                group_width = mode.w;
                group_height = mode.h;
            }

            if(ImGui::Selectable(choice_labels[i + 1].c_str(), current_index == choice_index)) {
                if(current_index != choice_index) {
                    should_open_video_mode_popup = true;
                    target_fullscreen = true;
                    target_width = mode.w;
                    target_height = mode.h;
                    target_rate = static_cast<int>(mode.refresh_rate);
                }
            }
        }

        ImGui::EndCombo();
    }

    layout_label();
    layout_tooltip();
}

static void on_keyboard_event(const SDL_KeyboardEvent& event)
{
    if(event.type == SDL_EVENT_KEY_DOWN && event.key == SDLK_ESCAPE && gui::screen == GUI_SETTINGS) {
        gui::screen = GUI_MAIN_MENU;
    }
}

static void on_language_update_event(const LanguageUpdateEvent& event)
{
    str_location_general = language::resolve_gui("settings.location.general");
    str_location_video = language::resolve_gui("settings.location.video");
    str_location_audio = language::resolve_gui("settings.location.audio");
    str_location_keyboard = language::resolve_gui("settings.location.keyboard");
    str_location_mouse = language::resolve_gui("settings.location.mouse");
    str_location_gamepad = language::resolve_gui("settings.location.gamepad");

    str_separator_settings = std::string(language::resolve("settings.separator.settings"));
    str_separator_devices = std::string(language::resolve("settings.separator.devices"));

    str_label_gui = std::string(language::resolve("settings.label.gui"));
    str_label_levels = std::string(language::resolve("settings.label.levels"));
    str_label_movement = std::string(language::resolve("settings.label.movement"));
    str_label_gameplay = std::string(language::resolve("settings.label.gameplay"));
    str_label_miscellaneous = std::string(language::resolve("settings.label.miscellaneous"));

    str_checkbox_false = std::string(language::resolve("settings.checkbox.false"));
    str_checkbox_true = std::string(language::resolve("settings.checkbox.true"));

    str_popup_video_change_title = language::resolve_gui("settings.popup.video_change.title");
    str_popup_video_change_text = std::string(language::resolve("settings.popup.video_change.text"));
    str_popup_video_change_choices[0] = language::resolve_gui("settings.popup.video_change.answer.yes");
    str_popup_video_change_choices[1] = language::resolve_gui("settings.popup.video_change.answer.no");

    for(auto& value : values_all) {
        value->refresh_locale();
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
    if(utils::selectable_button(label.c_str(), size, selected_location == target_location)) {
        selected_location = target_location;
    }
}

void settings::init(void)
{
    selected_location = gui_location::GENERAL;
    should_open_video_mode_popup = false;
    target_fullscreen = false;
    target_width = 0;
    target_height = 0;
    target_rate = 0;

    globals::dispatcher.sink<SDL_KeyboardEvent>().connect<&on_keyboard_event>();
    globals::dispatcher.sink<LanguageUpdateEvent>().connect<&on_language_update_event>();
}

void settings::init_late(void)
{
    for(std::size_t i = 0; i < NUM_LOCATIONS; ++i) {
        std::sort(values[i].begin(), values[i].end(), [](const auto& a, const auto& b) {
            return a->priority < b->priority;
        });
    }
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

    ImGui::PushFont(fonts::unscii16, 16.0f);
    ImGui::PushStyleVarX(ImGuiStyleVar_FramePadding, 8.0f * gui::scale);
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 3.2f * gui::scale);

    if(ImGui::Begin("###settings.window", nullptr, WINDOW_FLAGS)) {
        auto sidebar_width = 0.25f * window_size.x;

        if(ImGui::BeginChild("###settings.window.sidebar", ImVec2(sidebar_width, -1.0f), ImGuiChildFlags_AlwaysUseWindowPadding, 0)) {
            auto selectable_height = 24.0f * gui::scale;
            auto selectable_size = ImVec2(ImGui::GetContentRegionAvail().x, selectable_height);

            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

            if(ImGui::Button("<<###settings.window.back", ImVec2(-1.0f, 0.0f))) {
                gui::screen = GUI_MAIN_MENU;
            }

            ImGui::SeparatorText(str_separator_settings.c_str());
            layout_location_selectable(str_location_general, gui_location::GENERAL, selectable_size);
            layout_location_selectable(str_location_video, gui_location::VIDEO, selectable_size);
            layout_location_selectable(str_location_audio, gui_location::AUDIO, selectable_size);

            ImGui::SeparatorText(str_separator_devices.c_str());
            layout_location_selectable(str_location_keyboard, gui_location::KEYBOARD, selectable_size);
            layout_location_selectable(str_location_mouse, gui_location::MOUSE, selectable_size);
            layout_location_selectable(str_location_gamepad, gui_location::GAMEPAD, selectable_size);

            ImGui::PopStyleVar();
        }

        ImGui::EndChild();
        ImGui::SameLine();

        if(ImGui::BeginChild("###settings.window.contents", ImVec2(-1.0f, -1.0f), ImGuiChildFlags_AlwaysUseWindowPadding, 0)) {
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
                    layout_values(settings_location::SOUND);
                    ImGui::SeparatorText(str_label_levels.c_str());
                    layout_values(settings_location::SOUND_LEVELS);
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
                    layout_values(settings_location::GAMEPAD);
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

    auto popup_result = utils::popup(str_popup_video_change_title, str_popup_video_change_text, str_popup_video_change_choices);

    if(popup_result.has_value() && popup_result.value() == 0) {
        if(target_fullscreen) {
            video::request_fullscreen(target_width, target_height, target_rate);
        }
        else {
            video::request_windowed();
        }
    }

    ImGui::End();
}

void settings::checkbox(int priority, settings_location location, std::string_view key, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_CheckBox>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

template<>
void settings::input_arithmetic<float>(int priority, settings_location location, std::string_view key, float min, float max, bool tooltip,
    std::string_view fmt)
{
    auto setting_value = std::make_shared<SettingValue_InputFloat>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->min_value = min;
    setting_value->max_value = max;
    setting_value->format = fmt.empty() ? std::string("%.3f") : std::string(fmt);
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

template<>
void settings::input_arithmetic<unsigned>(int priority, settings_location location, std::string_view key, unsigned min, unsigned max,
    bool tooltip, std::string_view fmt)
{
    auto setting_value = std::make_shared<SettingValue_InputUnsigned>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->min_value = min;
    setting_value->max_value = max;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

template<>
void settings::input_arithmetic<int>(int priority, settings_location location, std::string_view key, int min, int max, bool tooltip,
    std::string_view fmt)
{
    auto setting_value = std::make_shared<SettingValue_InputInt>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->min_value = min;
    setting_value->max_value = max;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::input_string(int priority, settings_location location, std::string_view key, bool tooltip, bool allow_whitespace)
{
    auto setting_value = std::make_shared<SettingValue_InputString>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->allow_whitespace = allow_whitespace;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

template<>
void settings::slider<float>(int priority, settings_location location, std::string_view key, float min, float max, bool tooltip,
    std::string_view fmt)
{
    auto setting_value = std::make_shared<SettingValue_SliderFloat>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->min_value = min;
    setting_value->max_value = max;
    setting_value->format = fmt.empty() ? std::string("%.3f") : std::string(fmt);
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

template<>
void settings::slider<unsigned>(int priority, settings_location location, std::string_view key, unsigned min, unsigned max, bool tooltip,
    std::string_view fmt)
{
    auto setting_value = std::make_shared<SettingValue_SliderUnsigned>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->min_value = min;
    setting_value->max_value = max;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

template<>
void settings::slider<int>(int priority, settings_location location, std::string_view key, int min, int max, bool tooltip,
    std::string_view fmt)
{
    auto setting_value = std::make_shared<SettingValue_SliderInt>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->min_value = min;
    setting_value->max_value = max;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

template<>
void settings::stepper<unsigned>(int priority, settings_location location, std::string_view key, unsigned min, unsigned max, unsigned step,
    bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_StepperUnsigned>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->min_value = min;
    setting_value->max_value = max;
    setting_value->step_value = step;
    setting_value->wids.resize((max - min) / step + 1);

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

template<>
void settings::stepper<int>(int priority, settings_location location, std::string_view key, int min, int max, int step, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_StepperInt>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->min_value = min;
    setting_value->max_value = max;
    setting_value->step_value = step;
    setting_value->wids.resize((max - min) / step + 1);

    setting_value->value.bind(globals::client_config, key);
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::keybind(int priority, settings_location location, std::string_view key, bool tooltip)
{
    // TODO: config::KeyBind doesn't exist yet after the SDL3 port, keybind rebinding UI is not implemented
    assert(false && "settings::keybind not implemented");
}

void settings::language(int priority, settings_location location, std::string_view key, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_Language>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}

void settings::video_mode(int priority, settings_location location, std::string_view key, bool tooltip)
{
    auto setting_value = std::make_shared<SettingValue_VideoMode>();
    setting_value->priority = priority;
    setting_value->location = location;
    setting_value->key = key;
    setting_value->has_tooltip = tooltip;
    setting_value->wid = std::format("###{}", static_cast<const void*>(setting_value.get()));

    setting_value->refresh_choices();
    setting_value->refresh_locale();

    values[static_cast<unsigned int>(location)].push_back(setting_value);
    values_all.push_back(setting_value);
}
