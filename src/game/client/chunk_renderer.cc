#include "client/pch.hh"

#include "client/chunk_renderer.hh"

#include "core/config.hh"

#include "shared/chunk.hh"
#include "shared/coord.hh"
#include "shared/dimension.hh"

#include "client/camera.hh"
#include "client/chunk_mesher.hh"
#include "client/chunk_quad.hh"
#include "client/game.hh"
#include "client/globals.hh"
#include "client/outline.hh"
#include "client/program.hh"
#include "client/settings.hh"
#include "client/skybox.hh"
#include "client/toggles.hh"
#include "client/voxel_anims.hh"
#include "client/voxel_atlas.hh"

// ONLY TOUCH THESE IF THE RESPECTIVE SHADER
// VARIANT MACRO DECLARATIONS LAYOUT CHANGED AS WELL
constexpr static unsigned int WORLD_CURVATURE = 0U;
constexpr static unsigned int WORLD_FOG = 1U;

static ConfigBoolean depth_sort_chunks(true);

static GL_Program quad_program;
static std::size_t u_quad_vproj_matrix;
static std::size_t u_quad_world_position;
static std::size_t u_quad_timings;
static std::size_t u_quad_fog_color;
static std::size_t u_quad_view_distance;
static std::size_t u_quad_textures;
static GLuint quad_vaobj;
static GLuint quad_vbo;

void chunk_renderer::init(void)
{
    globals::client_config.add_value("chunk_renderer.depth_sort_chunks", depth_sort_chunks);

    settings::add_checkbox(5, depth_sort_chunks, settings_location::VIDEO, "chunk_renderer.depth_sort_chunks", false);

    if(!quad_program.setup("shaders/chunk_quad.vert", "shaders/chunk_quad.frag")) {
        spdlog::critical("chunk_renderer: quad_program: setup failed");
        std::terminate();
    }

    u_quad_vproj_matrix = quad_program.add_uniform("u_ViewProjMatrix");
    u_quad_world_position = quad_program.add_uniform("u_WorldPosition");
    u_quad_timings = quad_program.add_uniform("u_Timings");
    u_quad_fog_color = quad_program.add_uniform("u_FogColor");
    u_quad_view_distance = quad_program.add_uniform("u_ViewDistance");
    u_quad_textures = quad_program.add_uniform("u_Textures");

    const glm::fvec3 vertices[4] = {
        glm::fvec3(1.0f, 0.0f, 1.0f),
        glm::fvec3(1.0f, 0.0f, 0.0f),
        glm::fvec3(0.0f, 0.0f, 1.0f),
        glm::fvec3(0.0f, 0.0f, 0.0f),
    };

    glGenVertexArrays(1, &quad_vaobj);
    glBindVertexArray(quad_vaobj);

    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::fvec3), nullptr);
}

void chunk_renderer::shutdown(void)
{
    glDeleteBuffers(1, &quad_vbo);
    glDeleteVertexArrays(1, &quad_vaobj);
    quad_program.destroy();
}

void chunk_renderer::render(void)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glLineWidth(1.0f);

    if(toggles::get(TOGGLE_WIREFRAME)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    quad_program.set_variant_vert(WORLD_CURVATURE, client_game::world_curvature.get_value());
    quad_program.set_variant_vert(WORLD_FOG, client_game::fog_mode.get_value());
    quad_program.set_variant_frag(WORLD_FOG, client_game::fog_mode.get_value());

    if(!quad_program.update()) {
        spdlog::critical("chunk_renderer: quad_program: update failed");
        quad_program.destroy();
        std::terminate();
    }

    GLuint timings[3];
    timings[0] = globals::window_frametime;
    timings[1] = globals::window_frametime_avg;
    timings[2] = voxel_anims::frame;

    const auto group = globals::dimension->chunks.group<ChunkComponent>(entt::get<ChunkMeshComponent>);

    if(depth_sort_chunks.get_value()) {
        // FIXME: speed! sorting every frame doesn't look
        // like a good idea. Can we store the group elsewhere and
        // still have all the up-to-date chunk things inside?
        group.sort([](entt::entity ea, entt::entity eb) {
            const auto dir_a = globals::dimension->chunks.get<ChunkComponent>(ea).cpos - camera::position_chunk;
            const auto dir_b = globals::dimension->chunks.get<ChunkComponent>(eb).cpos - camera::position_chunk;

            const auto da = dir_a[0] * dir_a[0] + dir_a[1] * dir_a[1] + dir_a[2] * dir_a[2];
            const auto db = dir_b[0] * dir_b[0] + dir_b[1] * dir_b[1] + dir_b[2] * dir_b[2];

            return da > db;
        });
    }

    for(std::size_t plane_id = 0; plane_id < voxel_atlas::plane_count(); ++plane_id) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, voxel_atlas::plane_texture(plane_id));

        glBindVertexArray(quad_vaobj);

        glUseProgram(quad_program.handle);
        glUniformMatrix4fv(quad_program.uniforms[u_quad_vproj_matrix].location, 1, false, glm::value_ptr(camera::matrix));
        glUniform3uiv(quad_program.uniforms[u_quad_timings].location, 1, timings);
        glUniform4fv(quad_program.uniforms[u_quad_fog_color].location, 1, glm::value_ptr(skybox::fog_color));
        glUniform1f(quad_program.uniforms[u_quad_view_distance].location, camera::view_distance.get_value() * CHUNK_SIZE);
        glUniform1i(quad_program.uniforms[u_quad_textures].location, 0); // GL_TEXTURE0

        glDisable(GL_BLEND);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        for(const auto [entity, chunk, mesh] : group.each()) {
            if(plane_id < mesh.quad_nb.size() && mesh.quad_nb[plane_id].handle && mesh.quad_nb[plane_id].size) {
                const auto wpos = coord::to_fvec3(chunk.cpos - camera::position_chunk);
                glUniform3fv(quad_program.uniforms[u_quad_world_position].location, 1, glm::value_ptr(wpos));

                glBindBuffer(GL_ARRAY_BUFFER, mesh.quad_nb[plane_id].handle);

                glEnableVertexAttribArray(1);
                glVertexAttribDivisor(1, 1);
                glVertexAttribIPointer(1, 2, GL_UNSIGNED_INT, sizeof(ChunkQuad), nullptr);

                glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, mesh.quad_nb[plane_id].size);

                globals::num_drawcalls += 1;
                globals::num_triangles += 2 * mesh.quad_nb[plane_id].size;
            }
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for(const auto [entity, chunk, mesh] : group.each()) {
            if(plane_id < mesh.quad_b.size() && mesh.quad_b[plane_id].handle && mesh.quad_b[plane_id].size) {
                const auto wpos = coord::to_fvec3(chunk.cpos - camera::position_chunk);
                glUniform3fv(quad_program.uniforms[u_quad_world_position].location, 1, glm::value_ptr(wpos));

                glBindBuffer(GL_ARRAY_BUFFER, mesh.quad_b[plane_id].handle);

                glEnableVertexAttribArray(1);
                glVertexAttribDivisor(1, 1);
                glVertexAttribIPointer(1, 2, GL_UNSIGNED_INT, sizeof(ChunkQuad), nullptr);

                glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, mesh.quad_b[plane_id].size);

                globals::num_drawcalls += 1;
                globals::num_triangles += 2 * mesh.quad_b[plane_id].size;
            }
        }
    }

    if(toggles::get(TOGGLE_CHUNK_AABB)) {
        outline::prepare();

        for(const auto [entity, chunk, mesh] : group.each()) {
            const auto size = glm::fvec3(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);
            outline::cube(chunk.cpos, glm::fvec3(0.0f, 0.0f, 0.0f), size, 1.0f, glm::fvec4(1.0f, 1.0f, 0.0f, 1.0f));
        }
    }
}
