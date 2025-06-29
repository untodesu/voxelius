#ifndef CLIENT_CONST_HH
#define CLIENT_CONST_HH 1
#pragma once

#include "shared/const.hh"

// This key is then going to be reserved for only
// the debug toggles and users won't be able to
// use this key for conventional gameplay things
constexpr static int DEBUG_KEY = GLFW_KEY_F3;

constexpr static int BASE_WIDTH = 320;
constexpr static int BASE_HEIGHT = 240;

constexpr static int MIN_WIDTH = 2 * BASE_WIDTH;
constexpr static int MIN_HEIGHT = 2 * BASE_HEIGHT;

constexpr static int DEFAULT_WIDTH = 720;
constexpr static int DEFAULT_HEIGHT = 480;

static_assert(DEFAULT_WIDTH >= MIN_WIDTH);
static_assert(DEFAULT_HEIGHT >= MIN_HEIGHT);

constexpr static float MIN_PITCH = 0.0625f;
constexpr static float MAX_PITCH = 10.0f;

#endif // CLIENT_CONST_HH
