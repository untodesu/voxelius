#pragma once

namespace io
{
constexpr static int INVALID_GAMEPAD_AXIS = INT_MAX;
constexpr static int INVALID_GAMEPAD_BUTTON = INT_MAX;
} // namespace io

namespace config
{
class Boolean;
class Float;
} // namespace config

struct GLFWgamepadstate;

namespace io::gamepad
{
extern bool available;
extern config::Float deadzone;
extern config::Boolean active;
extern GLFWgamepadstate state;
extern GLFWgamepadstate last_state;
} // namespace io::gamepad

namespace io::gamepad
{
void init(void);
void update_late(void);
} // namespace io::gamepad

namespace io
{
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
} // namespace io
