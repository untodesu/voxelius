#include "client/pch.hh"
#include "client/outline.hh"

#include "core/config.hh"

#include "shared/coord.hh"

#include "client/camera.hh"
#include "client/const.hh"
#include "client/game.hh"
#include "client/program.hh"

// ONLY TOUCH THESE IF THE RESPECTIVE SHADER
// VARIANT MACRO DECLARATIONS LAYOUT CHANGED AS WELL
constexpr static unsigned int WORLD_CURVATURE = 0U;

static GL_Program program;
static std::size_t u_vpmatrix;
static std::size_t u_worldpos;
static std::size_t u_viewdist;
static std::size_t u_modulate;
static std::size_t u_scale;

static GLuint vaobj;
static GLuint cube_vbo;
static GLuint line_vbo;

void outline::init(void)
{
    if(!program.setup("shaders/outline.vert", "shaders/outline.frag")) {
        spdlog::critical("outline: program setup failed");
        std::terminate();
    }

    u_vpmatrix = program.add_uniform("u_ViewProjMatrix");
    u_worldpos = program.add_uniform("u_WorldPosition");
    u_viewdist = program.add_uniform("u_ViewDistance");
    u_modulate = program.add_uniform("u_Modulate");
    u_scale = program.add_uniform("u_Scale");

    const glm::fvec3 cube_vertices[24] = {
        glm::fvec3(0.0f, 0.0f, 0.0f), glm::fvec3(0.0f, 1.0f, 0.0f),
        glm::fvec3(0.0f, 1.0f, 0.0f), glm::fvec3(1.0f, 1.0f, 0.0f),
        glm::fvec3(1.0f, 1.0f, 0.0f), glm::fvec3(1.0f, 0.0f, 0.0f),
        glm::fvec3(1.0f, 0.0f, 0.0f), glm::fvec3(0.0f, 0.0f, 0.0f),

        glm::fvec3(0.0f, 0.0f, 1.0f), glm::fvec3(0.0f, 1.0f, 1.0f),
        glm::fvec3(0.0f, 1.0f, 1.0f), glm::fvec3(1.0f, 1.0f, 1.0f),
        glm::fvec3(1.0f, 1.0f, 1.0f), glm::fvec3(1.0f, 0.0f, 1.0f),
        glm::fvec3(1.0f, 0.0f, 1.0f), glm::fvec3(0.0f, 0.0f, 1.0f),

        glm::fvec3(0.0f, 0.0f, 0.0f), glm::fvec3(0.0f, 0.0f, 1.0f),
        glm::fvec3(0.0f, 1.0f, 0.0f), glm::fvec3(0.0f, 1.0f, 1.0f),
        glm::fvec3(1.0f, 0.0f, 0.0f), glm::fvec3(1.0f, 0.0f, 1.0f),
        glm::fvec3(1.0f, 1.0f, 0.0f), glm::fvec3(1.0f, 1.0f, 1.0f),
    };

    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    const glm::fvec3 line_vertices[2] = {
        glm::fvec3(0.0f, 0.0f, 0.0f),
        glm::fvec3(1.0f, 1.0f, 1.0f),
    };

    glGenBuffers(1, &line_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vaobj);

    glBindVertexArray(vaobj);
    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 0);
}

void outline::deinit(void)
{
    glDeleteVertexArrays(1, &vaobj);
    glDeleteBuffers(1, &line_vbo);
    glDeleteBuffers(1, &cube_vbo);
    program.destroy();
}

void outline::prepare(void)
{
    program.set_variant_vert(WORLD_CURVATURE, client_game::world_curvature.get_value());

    if(!program.update()) {
        spdlog::critical("outline_renderer: program update failed");
        std::terminate();
    }

    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(program.handle);
    glUniformMatrix4fv(program.uniforms[u_vpmatrix].location, 1, false, glm::value_ptr(camera::matrix));
    glUniform1f(program.uniforms[u_viewdist].location, CHUNK_SIZE * camera::view_distance.get_value());

    glBindVertexArray(vaobj);
    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 0);
}

void outline::cube(const chunk_pos &cpos, const glm::fvec3 &fpos, const glm::fvec3 &size, float thickness, const glm::fvec4 &color)
{
    auto patch_cpos = cpos - camera::position_chunk;

    glLineWidth(thickness);

    glUniform3fv(program.uniforms[u_worldpos].location, 1, glm::value_ptr(coord::to_fvec3(patch_cpos, fpos)));
    glUniform4fv(program.uniforms[u_modulate].location, 1, glm::value_ptr(color));
    glUniform3fv(program.uniforms[u_scale].location, 1, glm::value_ptr(size));

    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::fvec3), nullptr);
    glDrawArrays(GL_LINES, 0, 24);
}

void outline::line(const chunk_pos &cpos, const glm::fvec3 &fpos, const glm::fvec3 &size, float thickness, const glm::fvec4 &color)
{
    auto patch_cpos = cpos - camera::position_chunk;

    glLineWidth(thickness);

    glUniform3fv(program.uniforms[u_worldpos].location, 1, glm::value_ptr(coord::to_fvec3(patch_cpos, fpos)));
    glUniform4fv(program.uniforms[u_modulate].location, 1, glm::value_ptr(color));
    glUniform3fv(program.uniforms[u_scale].location, 1, glm::value_ptr(size));

    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::fvec3), nullptr);
    glDrawArrays(GL_LINES, 0, 2);
}
