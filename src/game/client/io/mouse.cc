// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: mouse.cc; Created: Tue Dec 30 2025 12:39:32
// Description: Mouse and scroll wheel handling

#include "client/pch.hh"

#include "client/io/mouse.hh"

#include "client/globals.hh"

static void on_cursor_enter_glfw(GLFWwindow* window, int entered)
{
    ImGui_ImplGlfw_CursorEnterCallback(window, entered);
}
static void on_cursor_pos_glfw(GLFWwindow* window, double xpos, double ypos)
{
    globals::dispatcher.trigger(CursorPosEvent(xpos, ypos));

    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
}

static void on_mouse_button_glfw(GLFWwindow* window, int button, int action, int modbits)
{
    globals::dispatcher.trigger(MouseButtonEvent(button, action, modbits));

    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, modbits);
}

static void on_scroll_glfw(GLFWwindow* window, double xoffset, double yoffset)
{
    globals::dispatcher.trigger(ScrollEvent(xoffset, yoffset));

    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void mouse::init(void)
{
    spdlog::info("mouse: taking over device events");
    glfwSetCursorEnterCallback(globals::window, &on_cursor_enter_glfw);
    glfwSetCursorPosCallback(globals::window, &on_cursor_pos_glfw);
    glfwSetMouseButtonCallback(globals::window, &on_mouse_button_glfw);
    glfwSetScrollCallback(globals::window, &on_scroll_glfw);
}
