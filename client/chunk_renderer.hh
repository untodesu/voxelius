#ifndef C4D8E1A2_7B3F_4C6D_9A8E_1F2B3C4D5E6F
#define C4D8E1A2_7B3F_4C6D_9A8E_1F2B3C4D5E6F

class Camera;

namespace chunk_renderer
{
void init(void);
void shutdown(void);
} // namespace chunk_renderer

namespace chunk_renderer
{
void upload(SDL_GPUCopyPass* copy_pass);
void render(SDL_GPURenderPass* render_pass);
} // namespace chunk_renderer

#endif /* C4D8E1A2_7B3F_4C6D_9A8E_1F2B3C4D5E6F */
