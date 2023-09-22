// SPDX-License-Identifier: MPL-2.0
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include <client/globals.hh>
#include <client/screen.hh>
#include <spdlog/spdlog.h>

static float calc_aspect(int width, int height)
{
    if(width > height)
        return static_cast<float>(width) / static_cast<float>(height);
    return static_cast<float>(height) / static_cast<float>(width);
}

static void on_framebuffer_size_event(GLFWwindow *window, int width, int height)
{
    globals::dispatcher.trigger(ScreenSizeEvent {
        .width = width, 
        .height = height,
        .aspect = calc_aspect(width, height),
    });
}

void screen::init()
{
    spdlog::debug("screen: taking over framebuffer events");
    glfwSetFramebufferSizeCallback(globals::window, &on_framebuffer_size_event);
}

void screen::init_late()
{
    int width, height;
    glfwGetFramebufferSize(globals::window, &width, &height);
    on_framebuffer_size_event(globals::window, width, height);
}

void screen::get_size(int &width, int &height)
{
    glfwGetFramebufferSize(globals::window, &width, &height);
}

void screen::get_size(float &width, float &height)
{
    int iwidth, iheight;
    glfwGetFramebufferSize(globals::window, &iwidth, &iheight);
    width = static_cast<float>(width);
    height = static_cast<float>(height);
}

float screen::get_aspect()
{
    int width, height;
    glfwGetFramebufferSize(globals::window, &width, &height);
    return calc_aspect(width, height);
}