#pragma once

#include "shared/globals.hh"

namespace config
{
class KeyBind;
class GamepadAxis;
class GamepadButton;
} // namespace config

namespace io
{
class ConfigMap;
} // namespace io

struct GLFWwindow;
struct ImFont;

namespace world
{
class Dimension;
} // namespace world

namespace globals
{
extern io::ConfigMap client_config;

extern GLFWwindow* window;

// Some gamesystems that aren't really
// gameplay-oriented might still use client
// framerate to interpolate discrete things
// so it's still a good idea to keep these available
extern float window_frametime;
extern float window_frametime_avg;
extern std::uint64_t window_frametime_us;
extern std::uint64_t window_framecount;

// https://gafferongames.com/post/fix_your_timestep/
extern std::uint64_t fixed_accumulator;

extern int width;
extern int height;
extern float aspect;

extern GLuint world_fbo;
extern GLuint world_fbo_color;
extern GLuint world_fbo_depth;

extern std::size_t num_drawcalls;
extern std::size_t num_triangles;

extern ENetHost* client_host;

extern world::Dimension* dimension;
extern entt::entity player;

extern ImFont* font_unscii16;
extern ImFont* font_unscii8;

extern config::KeyBind* gui_keybind_ptr;
extern config::GamepadAxis* gui_gamepad_axis_ptr;
extern config::GamepadButton* gui_gamepad_button_ptr;

extern unsigned int gui_scale;
extern unsigned int gui_screen;

extern ALCdevice* sound_dev;
extern ALCcontext* sound_ctx;
} // namespace globals
