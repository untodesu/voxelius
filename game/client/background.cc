// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2024, Voxelius Contributors
#include <common/image.hh>
#include <game/client/background.hh>
#include <game/client/globals.hh>
#include <game/client/varied_program.hh>
#include <mathlib/constexpr.hh>
#include <mathlib/vec2f.hh>
#include <spdlog/spdlog.h>

static VariedProgram program = {};
static std::size_t u_texture = {};
static std::size_t u_scale = {};
static GLuint vaobj = {};
static GLuint vbo = {};

static int texture_width = {};
static int texture_height = {};
static GLuint texture = {};

void background::init(void)
{
    if(!VariedProgram::setup(program, "shaders/background.vert", "shaders/background.frag")) {
        spdlog::critical("background: program setup failed");
        std::terminate();
    }

    u_scale = VariedProgram::add_uniform(program, "u_Scale");
    u_texture = VariedProgram::add_uniform(program, "u_Texture");

    const Vec2f vertices[4] = {
        Vec2f(0.0f, 1.0f),
        Vec2f(0.0f, 0.0f),
        Vec2f(1.0f, 1.0f),
        Vec2f(1.0f, 0.0f),
    };

    glGenVertexArrays(1, &vaobj);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(vaobj);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vec2f), nullptr);
    glVertexAttribDivisor(0, 0);

    Image image = {};
    if(!Image::load_rgba(image, std::string("textures/gui/background.png"), true)) {
        std::terminate();
    }

    texture_width = image.width;
    texture_height = image.height;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    Image::unload(image);
}

void background::deinit(void)
{
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &vaobj);
    glDeleteBuffers(1, &vbo);
    VariedProgram::destroy(program);
}

void background::render(void)
{
    if(!VariedProgram::update(program)) {
        spdlog::critical("background: program update failed");
        std::terminate();
    }

    const float scaled_width = 0.5f * static_cast<float>(globals::width / globals::gui_scale);
    const float scaled_height = 0.5f * static_cast<float>(globals::height / globals::gui_scale);
    const float scale_x = scaled_width / static_cast<float>(texture_width);
    const float scale_y = scaled_height / static_cast<float>(texture_height);

    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUseProgram(program.handle);
    glUniform2f(program.uniforms[u_scale].location, scale_x, scale_y);
    glUniform1i(program.uniforms[u_texture].location, 0); // GL_TEXTURE0

    glBindVertexArray(vaobj);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
