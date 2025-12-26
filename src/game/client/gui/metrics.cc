#include "client/pch.hh"

#include "client/gui/metrics.hh"

#include "core/version.hh"

#include "shared/entity/grounded.hh"
#include "shared/entity/head.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"

#include "shared/coord.hh"

#include "client/entity/camera.hh"

#include "client/gui/imutils_text.hh"

#include "client/game.hh"
#include "client/globals.hh"
#include "client/session.hh"

constexpr static ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs
    | ImGuiWindowFlags_NoNav;

static std::basic_string<GLubyte> r_version;
static std::basic_string<GLubyte> r_renderer;

void metrics::init(void)
{
    r_version = std::basic_string<GLubyte>(glGetString(GL_VERSION));
    r_renderer = std::basic_string<GLubyte>(glGetString(GL_RENDERER));
}

void metrics::layout(void)
{
    if(!session::is_ingame()) {
        // Sanity check; we are checking this
        // in client_game before calling layout
        // on HUD-ish GUI systems but still
        return;
    }

    auto draw_list = ImGui::GetForegroundDrawList();

    // FIXME: maybe use style colors instead of hardcoding?
    auto text_color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    auto shadow_color = ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    auto font_size = 8.0f * globals::gui_scale;
    auto position = ImVec2(8.0f, 8.0f);
    auto y_step = 1.5f * font_size;

    // Draw version
    auto version_line = std::format("Voxelius {}", version::full);
    imutils::text_nw(draw_list, version_line, position, text_color, shadow_color, globals::font_unscii8, font_size);
    position.y += 1.5f * y_step;

    // Draw client-side window framerate metrics
    auto window_framerate = 1.0f / globals::window_frametime_avg;
    auto window_frametime = 1000.0f * globals::window_frametime_avg;
    auto window_fps_line = std::format("{:.02f} FPS [{:.02f} ms]", window_framerate, window_frametime);
    imutils::text_nw(draw_list, window_fps_line, position, text_color, shadow_color, globals::font_unscii8, font_size);
    position.y += y_step;

    // Draw world rendering metrics
    auto drawcall_line = std::format("World: {} DC / {} TRI", globals::num_drawcalls, globals::num_triangles);
    imutils::text_nw(draw_list, drawcall_line, position, text_color, shadow_color, globals::font_unscii8, font_size);
    position.y += y_step;

    // Draw OpenGL version string
    auto r_version_line = std::format("GL_VERSION: {}", reinterpret_cast<const char*>(r_version.c_str()));
    imutils::text_nw(draw_list, r_version_line, position, text_color, shadow_color, globals::font_unscii8, font_size);
    position.y += y_step;

    // Draw OpenGL renderer string
    auto r_renderer_line = std::format("GL_RENDERER: {}", reinterpret_cast<const char*>(r_renderer.c_str()));
    imutils::text_nw(draw_list, r_renderer_line, position, text_color, shadow_color, globals::font_unscii8, font_size);
    position.y += 1.5f * y_step;

    const auto& head = globals::dimension->entities.get<Head>(globals::player);
    const auto& transform = globals::dimension->entities.get<Transform>(globals::player);
    const auto& velocity = globals::dimension->entities.get<Velocity>(globals::player);

    // Draw player voxel position
    auto voxel_position = coord::to_voxel(transform.chunk, transform.local);
    auto voxel_line = std::format("voxel: [{} {} {}]", voxel_position.x, voxel_position.y, voxel_position.z);
    imutils::text_nw(draw_list, voxel_line, position, text_color, shadow_color, globals::font_unscii8, font_size);
    position.y += y_step;

    // Draw player world position
    auto world_line = std::format("world: [{} {} {}] [{:.03f} {:.03f} {:.03f}]", transform.chunk.x, transform.chunk.y, transform.chunk.z,
        transform.local.x, transform.local.y, transform.local.z);
    imutils::text_nw(draw_list, world_line, position, text_color, shadow_color, globals::font_unscii8, font_size);
    position.y += y_step;

    // Draw player look angles
    auto angles = glm::degrees(transform.angles + head.angles);
    auto angle_line = std::format("angle: [{: .03f} {: .03f} {: .03f}]", angles[0], angles[1], angles[2]);
    imutils::text_nw(draw_list, angle_line, position, text_color, shadow_color, globals::font_unscii8, font_size);
    position.y += y_step;
}
