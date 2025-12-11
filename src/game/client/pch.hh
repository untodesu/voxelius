#pragma once

#include <shared/pch.hh>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/salad.h>

#include <dr_mp3.h>
#include <dr_wav.h>

#include <GLFW/glfw3.h>

#include <glad/gl.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(__unix__)
#include <dlfcn.h>
#endif
