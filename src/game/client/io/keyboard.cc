// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: keyboard.cc; Created: Tue Dec 30 2025 12:27:50
// Description: Keyboard handling

#include "client/pch.hh"

#include "client/io/keyboard.hh"

#include "client/globals.hh"

static void on_char_glfw(GLFWwindow* window, unsigned int codepoint)
{
    ImGui_ImplGlfw_CharCallback(window, codepoint);
}

static void on_key_glfw(GLFWwindow* window, int keycode, int scancode, int action, int modbits)
{
    globals::dispatcher.trigger(KeyEvent(keycode, scancode, action, modbits));

    ImGui_ImplGlfw_KeyCallback(window, keycode, scancode, action, modbits);
}

void keyboard::init(void)
{
    spdlog::info("keyboard: taking over device events");
    glfwSetCharCallback(globals::window, &on_char_glfw);
    glfwSetKeyCallback(globals::window, &on_key_glfw);
}
