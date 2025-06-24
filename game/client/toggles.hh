#ifndef CLIENT_TOGGLES_HH
#define CLIENT_TOGGLES_HH 1
#pragma once

using toggle_type = unsigned int;
constexpr static toggle_type TOGGLE_WIREFRAME = 0x0000U;   // Render things in wireframe mode
constexpr static toggle_type TOGGLE_FULLBRIGHT = 0x0001U;  // Render things without lighting
constexpr static toggle_type TOGGLE_CHUNK_AABB = 0x0002U;  // Render chunk bounding boxes
constexpr static toggle_type TOGGLE_METRICS_UI = 0x0003U;  // Render debug metrics overlay
constexpr static toggle_type TOGGLE_USE_GAMEPAD = 0x0004U; // Use gamepad for player movement
constexpr static toggle_type TOGGLE_PM_FLIGHT = 0x0005U;   // Enable flight for player movement
constexpr static std::size_t TOGGLE_COUNT = 0x0006U;

struct ToggleEnabledEvent final {
    toggle_type type;
};

struct ToggleDisabledEvent final {
    toggle_type type;
};

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
void init(void);
void init_late(void);
bool get(toggle_type type);
void set(toggle_type type, bool value);
} // namespace toggles

#endif /* CLIENT_TOGGLES_HH */
