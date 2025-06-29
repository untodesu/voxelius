#ifndef CLIENT_GAMEPAD_HH
#define CLIENT_GAMEPAD_HH 1
#pragma once

constexpr static int INVALID_GAMEPAD_AXIS = INT_MAX;
constexpr static int INVALID_GAMEPAD_BUTTON = INT_MAX;

class ConfigBoolean;
class ConfigFloat;

struct GLFWgamepadstate;

namespace gamepad
{
extern bool available;
extern ConfigFloat deadzone;
extern ConfigBoolean active;
extern GLFWgamepadstate state;
extern GLFWgamepadstate last_state;
} // namespace gamepad

namespace gamepad
{
void init(void);
void update_late(void);
} // namespace gamepad

// This simulates buttons using axes. When an axis
// value exceeds 1.5 times the deadzone, the event is
// queued with a GLFW_PRESS action, when it falls back
// below the threshold, the event is queued with GLFW_RELEASE action
struct GamepadAxisEvent final {
    int action;
    int axis;
};

// This smears GLFW event sugar over gamepad polling
// system. Whenever it detects a state change, the event
// is queued with an appropriate action
struct GamepadButtonEvent final {
    int action;
    int button;
};

#endif /* CLIENT_GAMEPAD_HH  */
