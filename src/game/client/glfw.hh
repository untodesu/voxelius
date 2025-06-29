#ifndef CLIENTFW
#define CLIENTFW 1
#pragma once

struct GlfwCursorPosEvent final {
    glm::fvec2 pos;
};

struct GlfwFramebufferSizeEvent final {
    glm::ivec2 size;
    float aspect;
};

struct GlfwJoystickEvent final {
    int joystick_id;
    int event_type;
};

struct GlfwKeyEvent final {
    int key { GLFW_KEY_UNKNOWN };
    int scancode;
    int action;
    int mods;
};

struct GlfwMouseButtonEvent final {
    int button { GLFW_KEY_UNKNOWN };
    int action;
    int mods;
};

struct GlfwScrollEvent final {
    float dx;
    float dy;
};

#endif /* CLIENTFW */
