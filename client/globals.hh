#ifndef D618E26B_01F4_4D5C_A249_6D2B49637425
#define D618E26B_01F4_4D5C_A249_6D2B49637425

#include "shared/globals.hh"

namespace config
{
class Map;
} // namespace config

namespace globals
{
extern config::Map client_config;
} // namespace globals

namespace globals
{
extern SDL_Window* window;
extern SDL_GPUDevice* gpu_device;
} // namespace globals

namespace globals
{
extern SDL_GPUCommandBuffer* gpu_commands_main;
extern SDL_GPUTexture* gpu_swapchain;
extern SDL_GPUTexture* gpu_depth_stencil;
} // namespace globals

namespace globals
{
extern int width;
extern int height;
extern float aspect;
} // namespace globals

namespace globals
{
extern std::uint64_t fixed_accumulator_us;
extern std::size_t window_framecount;
extern std::uint64_t window_frametime_us;
extern float window_frametime;
extern float window_frametime_avg;
} // namespace globals

namespace globals
{
extern std::size_t num_draw_calls;
extern std::size_t num_draw_vertices;
} // namespace globals

namespace globals
{
extern entt::entity player;
} // namespace globals

#endif /* D618E26B_01F4_4D5C_A249_6D2B49637425 */
