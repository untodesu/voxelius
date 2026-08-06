#include "client/pch.hh"

#include "client/game.hh"

#include "core/camera.hh"
#include "core/identifier.hh"
#include "core/utils/angles.hh"

#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/physics/physics.hh"
#include "shared/utils/coord.hh"
#include "shared/utils/world.hh"
#include "shared/world/block_registry.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/gui/background.hh"
#include "client/gui/button.hh"
#include "client/gui/checkbox.hh"
#include "client/gui/dimmer.hh"
#include "client/gui/input.hh"
#include "client/gui/keybind.hh"
#include "client/gui/language_selector.hh"
#include "client/gui/menu.hh"
#include "client/gui/popup.hh"
#include "client/gui/screen.hh"
#include "client/gui/scroller.hh"
#include "client/gui/slider.hh"
#include "client/gui/stack.hh"
#include "client/gui/stepper.hh"
#include "client/gui/title.hh"
#include "client/gui/video_selector.hh"
#include "client/language.hh"
#include "client/net/bother.hh"
#include "client/system/interpolation.hh"
#include "client/system/player_look.hh"
#include "client/system/player_move.hh"
#include "client/system/player_target.hh"

// Test screens for the new gui:: widget tree; will be
// replaced by the real main menu/settings once they're ported over.
static gui::Screen s_menu_screen {};
static gui::Background s_menu_background {};
static gui::Title s_menu_title {};
static gui::Menu s_menu_menu {};
static gui::Popup s_menu_popup {};

static gui::Screen s_settings_screen {};
static gui::Background s_settings_background {};
static gui::Dimmer s_settings_dimmer {};
static gui::HorizontalStack s_settings_hstack {};
static gui::Button s_settings_back {};
static gui::Scroller s_settings_scroll {};

static gui::CheckBox s_test_checkbox {};
static gui::SliderInt s_test_slider_int {};
static gui::SliderFloat s_test_slider_float {};
static gui::SliderUnsigned s_test_slider_unsigned {};
static gui::StepperInt s_test_stepper_int {};
static gui::StepperUnsigned s_test_stepper_unsigned {};
static gui::InputInt s_test_input_int {};
static gui::InputFloat s_test_input_float {};
static gui::InputUnsigned s_test_input_unsigned {};
static gui::KeyBind s_test_keybind {};
static gui::LanguageSelector s_test_language {};
static gui::VideoSelector s_test_video {};

static void on_bother_response(const BotherResponseEvent& event)
{
    if(event.unreachable()) {
        LOG_WARNING("bother test: request {} unreachable", event.request_id());
        return;
    }

    LOG_INFO("bother test: request {} -> {}.{}.{}, {}/{} players, motd: \"{}\"", event.request_id(), event.version_major(),
        event.version_minor(), event.version_patch(), event.num_players(), event.max_players(), event.motd());
}

static void on_language_update_event(const LanguageUpdateEvent&)
{
    s_menu_screen.translate();
    s_settings_screen.translate();
}

static void on_keyboard_event(const SDL_KeyboardEvent& event)
{
    if(event.type != SDL_EVENT_KEY_DOWN || event.key != SDLK_ESCAPE) {
        return;
    }

    if(globals::gui_screen) {
        globals::gui_screen = nullptr;
    }
    else {
        globals::gui_screen = &s_menu_screen;
    }
}

void client_game::init(void)
{
    interpolation::init();

    gui::KeyBind::init();

    player_look::init();
    player_move::init();
    player_target::init();

    // --- main test menu ---
    s_menu_screen.set_start(0.0f, 0.0f);
    s_menu_screen.set_size(1.0f, 1.0f);

    s_menu_title.set_text("test.title");

    s_menu_menu.set_margin(ImVec2(16.0f, 16.0f));
    s_menu_menu.set_control(ImVec2(256.0f, 32.0f));
    s_menu_menu.add_button("test.menu.settings", [] {
        globals::gui_screen = &s_settings_screen;
    });
    s_menu_menu.add_button("test.menu.popup", [] {
        s_menu_popup.open();
    });
    s_menu_menu.add_button("test.menu.quit", [] {
        LOG_INFO("quit clicked (test only, not wired up)");
    });

    s_menu_popup.set_title("test.popup.title");
    s_menu_popup.set_question("test.popup.question");
    s_menu_popup.add_choice("test.popup.ok");

    s_menu_screen.add_child(s_menu_background);
    s_menu_screen.add_child(s_menu_title);
    s_menu_screen.add_child(s_menu_menu);
    s_menu_screen.add_child(s_menu_popup);
    s_menu_screen.translate();

    // --- settings test screen ---
    s_settings_screen.set_start(0.0f, 0.0f);
    s_settings_screen.set_size(1.0f, 1.0f);

    s_settings_back.set_text("test.settings.back");
    s_settings_back.set_size(-1.0f, -1.0f);
    s_settings_back.on_click([] {
        globals::gui_screen = &s_menu_screen;
    });

    s_settings_scroll.set_margin(ImVec2(8.0f, 8.0f));

    s_test_checkbox.bind(globals::client_config, "test.checkbox");
    s_test_checkbox.enable_tooltip();

    s_test_slider_int.bind(globals::client_config, "test.slider_int");
    s_test_slider_int.set_range(0, 100);

    s_test_slider_float.bind(globals::client_config, "test.slider_float");
    s_test_slider_float.set_range(0.0f, 1.0f);

    s_test_slider_unsigned.bind(globals::client_config, "test.slider_unsigned");
    s_test_slider_unsigned.set_range(1u, 32u);

    s_test_stepper_int.bind(globals::client_config, "test.stepper_int");
    s_test_stepper_int.set_range(0, 4, 1);

    s_test_stepper_unsigned.bind(globals::client_config, "test.stepper_unsigned");
    s_test_stepper_unsigned.set_range(0u, 2u, 1u);

    s_test_input_int.bind(globals::client_config, "test.input_int");
    s_test_input_int.set_range(-100, 100);

    s_test_input_float.bind(globals::client_config, "test.input_float");
    s_test_input_float.set_range(0.0f, 10.0f);

    s_test_input_unsigned.bind(globals::client_config, "test.input_unsigned");
    s_test_input_unsigned.set_range(0u, 64u);

    s_test_keybind.bind(globals::client_config, "test.keybind");

    s_test_language.set_key("language");

    s_test_video.set_key("video.current_mode");
    s_test_video.on_change([](const gui::VideoSelector::mode_type&) -> gui::VideoSelector::decision_type {
        // Тестово применяем сразу, без диалога подтверждения
        return true;
    });

    s_settings_scroll.add_child(s_test_checkbox);
    s_settings_scroll.add_child(s_test_slider_int);
    s_settings_scroll.add_child(s_test_slider_float);
    s_settings_scroll.add_child(s_test_slider_unsigned);
    s_settings_scroll.add_child(s_test_stepper_int);
    s_settings_scroll.add_child(s_test_stepper_unsigned);
    s_settings_scroll.add_child(s_test_input_int);
    s_settings_scroll.add_child(s_test_input_float);
    s_settings_scroll.add_child(s_test_input_unsigned);
    s_settings_scroll.add_child(s_test_keybind);
    s_settings_scroll.add_child(s_test_language);
    s_settings_scroll.add_child(s_test_video);

    s_settings_hstack.add_item(s_settings_back, gui::FIXED, 220.0f);
    s_settings_hstack.add_item(s_settings_scroll, gui::EXPANDING);

    s_settings_screen.add_child(s_settings_background);
    s_settings_screen.add_child(s_settings_dimmer);
    s_settings_screen.add_child(s_settings_hstack);
    s_settings_screen.translate();

    globals::dispatcher.sink<SDL_KeyboardEvent>().connect<&on_keyboard_event>();
    globals::dispatcher.sink<LanguageUpdateEvent>().connect<&on_language_update_event>();
    globals::dispatcher.sink<BotherResponseEvent>().connect<&on_bother_response>();

    bother::ping(1, "127.0.0.1", 16384);
}

void client_game::init_late(void)
{
    // TODO: spawn the local player entity through the entities/class system
}

void client_game::shutdown(void)
{
    // empty
}

void client_game::update(void)
{
    ZoneScoped;

    interpolation::update();

    player_target::update();
}

void client_game::update_late(void)
{
    ZoneScoped;

    player_look::update_late();
    player_move::update_late();
}

void client_game::fixed_update(void)
{
    ZoneScoped;

    player_move::fixed_update();
}

void client_game::fixed_update_late(void)
{
    ZoneScoped;

    // empty
}

void client_game::layout(void)
{
    // empty
}

void client_game::render(void)
{
    ZoneScoped;

    player_target::render();
}
