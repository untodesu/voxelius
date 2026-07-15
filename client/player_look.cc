#include "client/pch.hh"

#include "client/player_look.hh"

#include "core/config/map.hh"
#include "core/config/ref.hh"
#include "core/utils/angles.hh"

#include "shared/head.hh"
#include "shared/world.hh"

#include "client/constant.hh"
#include "client/globals.hh"
#include "client/gui.hh"
#include "client/settings.hh"

constexpr static float PITCH_MIN = -1.0f * utils::radians(90.0f);
constexpr static float PITCH_MAX = +1.0f * utils::radians(90.0f);

static config::Ref<unsigned> s_sensitivity { 100 };

static void add_angles(float pitch, float yaw)
{
    if(world::basic_entities.valid(globals::player)) {
        auto& head = world::basic_entities.get<Head>(globals::player);

        head.angles[0] += pitch;
        head.angles[1] += yaw;
        head.angles[0] = std::clamp(head.angles[0], PITCH_MIN, PITCH_MAX);
        head.angles = utils::wrap_angle_180(head.angles);

        // Client-side head angles are not interpolated; re-assigning
        // the previous state after updating the current one is something
        // of a way to force the interpolation to behave like we need it to
        world::basic_entities.emplace_or_replace<Head_Prev>(globals::player, head);
    }
}

static void on_mouse_motion(const SDL_MouseMotionEvent& event)
{
    if(gui::screen || !world::basic_entities.valid(globals::player)) {
        return;
    }

    auto dx = -0.01f * static_cast<float>(s_sensitivity.value()) * utils::radians(event.xrel);
    auto dy = -0.01f * static_cast<float>(s_sensitivity.value()) * utils::radians(event.yrel);
    add_angles(dy, dx);
}

void player_look::init(void)
{
    s_sensitivity.bind(globals::client_config, "player_look.sensitivity");

    settings::slider(0, settings_location::MOUSE, "player_look.sensitivity", 25, 100, true);

    globals::dispatcher.sink<SDL_MouseMotionEvent>().connect<&on_mouse_motion>();
}

void player_look::update_late(void)
{
    auto& io = ImGui::GetIO();

    if(gui::screen) {
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        SDL_SetWindowRelativeMouseMode(globals::window, false);
    }
    else {
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        SDL_SetWindowRelativeMouseMode(globals::window, true);
    }
}
