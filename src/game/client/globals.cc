#include "client/pch.hh"

#include "client/globals.hh"

#include "core/config.hh"

#include "client/gui_screen.hh"

Config globals::client_config;

GLFWwindow* globals::window;

float globals::window_frametime;
float globals::window_frametime_avg;
std::uint64_t globals::window_frametime_us;
std::uint64_t globals::window_framecount;

std::uint64_t globals::fixed_accumulator;

int globals::width;
int globals::height;
float globals::aspect;

GLuint globals::world_fbo;
GLuint globals::world_fbo_color;
GLuint globals::world_fbo_depth;

std::size_t globals::num_drawcalls;
std::size_t globals::num_triangles;

ENetHost* globals::client_host;

Dimension* globals::dimension = nullptr;
entt::entity globals::player;

ImFont* globals::font_debug;
ImFont* globals::font_default;
ImFont* globals::font_chat;

ConfigKeyBind* globals::gui_keybind_ptr = nullptr;
ConfigGamepadAxis* globals::gui_gamepad_axis_ptr = nullptr;
ConfigGamepadButton* globals::gui_gamepad_button_ptr = nullptr;

unsigned int globals::gui_scale = 0U;
unsigned int globals::gui_screen = GUI_SCREEN_NONE;

ALCdevice* globals::sound_dev;
ALCcontext* globals::sound_ctx;
