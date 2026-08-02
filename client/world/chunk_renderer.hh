#ifndef C4D8E1A2_7B3F_4C6D_9A8E_1F2B3C4D5E6F
#define C4D8E1A2_7B3F_4C6D_9A8E_1F2B3C4D5E6F

namespace chunk_renderer
{
void init(void);
void shutdown(void);
void prepare(void);
void render_opaque(void);
void render_alpha(void);
void render_fluid(void);
} // namespace chunk_renderer

#endif /* C4D8E1A2_7B3F_4C6D_9A8E_1F2B3C4D5E6F */
