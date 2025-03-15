#ifndef CLIENT_TOGGLES_HH
#define CLIENT_TOGGLES_HH 1
#pragma once

namespace toggles
{
// The value is true whenever the debug
// toggles manager awaits for a sequenced key
// to be pressed. During this no input should
// be processed by any other gameplay system
extern bool is_sequence_await;
} // namespace toggles

namespace toggles
{
extern bool draw_chunk_borders;
extern bool draw_metrics;
extern bool render_fullbright;
extern bool render_wireframe;
} // namespace toggles

namespace toggles
{
void init(void);
} // namespace toggles

#endif /* CLIENT_TOGGLES_HH */
