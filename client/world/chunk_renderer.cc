#include "client/pch.hh"

#include "client/world/chunk_renderer.hh"

#include "core/camera.hh"
#include "core/config/map.hh"
#include "core/config/ref.hh"
#include "core/exception.hh"
#include "core/threading.hh"

#include "shared/constant.hh"
#include "shared/utils/chunk.hh"
#include "shared/utils/coord.hh"
#include "shared/world/chunk.hh"
#include "shared/world/world.hh"

#include "client/entity/camera.hh"
#include "client/fog.hh"
#include "client/globals.hh"
#include "client/shader_program.hh"
#include "client/world/block_atlas.hh"
#include "client/world/chunk_mesh.hh"
#include "client/world/skybox.hh"

// ONLY TOUCH THESE IF THE RESPECTIVE SHADER
// VARIANT MACRO DECLARATIONS LAYOUT CHANGED AS WELL
constexpr static unsigned FOG_MODEL = 0U;

static config::Ref<unsigned> s_fog_model;

static ShaderProgram s_program;
static std::size_t su_ViewProjection;
static std::size_t su_AnimationTimer;
static std::size_t su_WorldPosition;
static std::size_t su_ViewDistance;
static std::size_t su_FogColor;
static std::size_t su_AtlasTexture;
static std::size_t su_AtlasStrips;
static std::size_t su_AtlasFrames;

static GLuint s_vao;

static bool s_sorted_dirty;
static std::vector<std::pair<entt::entity, float>> s_sorted_opaque;
static std::vector<std::pair<entt::entity, float>> s_sorted_alpha;
static std::vector<std::pair<entt::entity, float>> s_sorted_fluid;

static ChunkPos s_last_camera_chunk;

static void draw_part(const Chunk_Component& chunk, const ChunkMesh_Part& part)
{
    if(part.count == 0 || part.vbo == 0) {
        return;
    }

    glUniform3fv(s_program.uniforms[su_WorldPosition].location, 1, utils::to_fvec(chunk.position - camera::chunk).data());

    glBindBuffer(GL_ARRAY_BUFFER, part.vbo);

    auto stride = static_cast<GLsizei>(sizeof(ChunkMesh_Quad));
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_origin)));
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_edge_u)));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_edge_v)));
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_uv)));
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_texture)));
    glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_extras)));

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, part.count);
}

static float chunk_distance_sq(entt::entity entity)
{
    const auto& chunk = world::chunk_entities.get<Chunk_Component>(entity);
    Eigen::Vector3f position = utils::to_fvec(chunk.position - camera::chunk) * static_cast<float>(constant::CHUNK_SIZE);
    Eigen::Vector3f delta = position - camera::local;
    return delta.squaredNorm();
}

static void update_sorted_chunks(void)
{
    const auto& frustum = camera::instance.frustum();

    auto view = world::chunk_entities.view<Chunk_Component, ChunkMesh>();

    s_sorted_opaque.clear();
    s_sorted_opaque.reserve(view.size_hint());

    for(const auto [entity, chunk, mesh] : view.each()) {
        if(mesh.opaque.count && mesh.opaque.vbo) {
            s_sorted_opaque.push_back(std::make_pair(entity, chunk_distance_sq(entity)));
        }
    }

    std::sort(s_sorted_opaque.begin(), s_sorted_opaque.end(), [&](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    s_sorted_alpha.clear();
    s_sorted_alpha.reserve(view.size_hint());

    for(const auto [entity, chunk, mesh] : view.each()) {
        if(mesh.alpha.vbo && mesh.alpha.count) {
            s_sorted_alpha.push_back(std::make_pair(entity, chunk_distance_sq(entity)));
        }
    }

    std::sort(s_sorted_alpha.begin(), s_sorted_alpha.end(), [&](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    s_sorted_fluid.clear();
    s_sorted_fluid.reserve(view.size_hint());

    for(const auto [entity, chunk, mesh] : view.each()) {
        if(mesh.fluid.vbo && mesh.fluid.count) {
            s_sorted_fluid.push_back(std::make_pair(entity, chunk_distance_sq(entity)));
        }
    }

    std::sort(s_sorted_fluid.begin(), s_sorted_fluid.end(), [&](const auto& a, const auto& b) {
        return a.second > b.second;
    });
}

static void on_chunk_create(const ChunkCreateEvent& event)
{
    s_sorted_dirty = true;
}

static void on_chunk_remove(const ChunkRemoveEvent& event)
{
    s_sorted_dirty = true;
}

static void on_chunk_update(const ChunkUpdateEvent& event)
{
    s_sorted_dirty = true;
}

static void on_chunk_mesh(entt::entity entity)
{
    s_sorted_dirty = true;
}

void chunk_renderer::init(void)
{
    s_fog_model.bind(globals::client_config, "head.fog_model");

    auto program_id = Identifier::from_parts(constant::BUILTIN_NAME_SPACE, "chunk");
    auto program_ok = s_program.setup(program_id);
    vx::throw_if_not_fmt(program_ok, "{}: setup failed", program_id.full_string());

    su_ViewProjection = s_program.add_uniform("u_ViewProjection");
    su_AnimationTimer = s_program.add_uniform("u_AnimationTimer");
    su_WorldPosition = s_program.add_uniform("u_WorldPosition");
    su_ViewDistance = s_program.add_uniform("u_ViewDistance");
    su_FogColor = s_program.add_uniform("u_FogColor");
    su_AtlasTexture = s_program.add_uniform("u_AtlasTexture");
    su_AtlasStrips = s_program.add_uniform("u_AtlasStrips");
    su_AtlasFrames = s_program.add_uniform("u_AtlasFrames");

    glGenVertexArrays(1, &s_vao);
    glBindVertexArray(s_vao);

    for(GLuint i = 0; i < 6; ++i) {
        glEnableVertexAttribArray(i);
        glVertexAttribDivisor(i, 1);
    }

    s_sorted_dirty = true;
    s_sorted_opaque.clear();
    s_sorted_alpha.clear();
    s_sorted_fluid.clear();

    s_last_camera_chunk = camera::chunk + ChunkPos::Ones();

    globals::dispatcher.sink<ChunkCreateEvent>().connect<&on_chunk_create>();
    globals::dispatcher.sink<ChunkRemoveEvent>().connect<&on_chunk_remove>();
    globals::dispatcher.sink<ChunkUpdateEvent>().connect<&on_chunk_update>();
    world::chunk_entities.on_update<ChunkMesh>().connect<&on_chunk_mesh>();
}

void chunk_renderer::shutdown(void)
{
    glDeleteVertexArrays(1, &s_vao);

    su_AtlasFrames = std::numeric_limits<std::size_t>::max();
    su_AtlasStrips = std::numeric_limits<std::size_t>::max();
    su_AtlasTexture = std::numeric_limits<std::size_t>::max();
    su_WorldPosition = std::numeric_limits<std::size_t>::max();
    su_AnimationTimer = std::numeric_limits<std::size_t>::max();
    su_ViewProjection = std::numeric_limits<std::size_t>::max();
    s_program.destroy();
}

void chunk_renderer::render(void)
{
    if(block_atlas::texture == 0 || block_atlas::tbo_frames == 0 || block_atlas::tbo_strips == 0) {
        return;
    }

    if(s_sorted_dirty || !camera::chunk.isApprox(s_last_camera_chunk)) {
        s_last_camera_chunk = camera::chunk;
        s_sorted_dirty = false;
        update_sorted_chunks();
    }

    s_program.set_variant_vert(FOG_MODEL, s_fog_model.value());
    s_program.set_variant_frag(FOG_MODEL, s_fog_model.value());

    vx::throw_if_not(s_program.update());

    const auto& vproj = camera::instance.view_projection();
    const auto& frustum = camera::instance.frustum();

    auto animation_timer = static_cast<std::uint32_t>(world::current_tick >> 2);

    glUseProgram(s_program.handle);
    glUniformMatrix4fv(s_program.uniforms[su_ViewProjection].location, 1, GL_FALSE, vproj.data());
    glUniform1ui(s_program.uniforms[su_AnimationTimer].location, animation_timer);
    glUniform1f(s_program.uniforms[su_ViewDistance].location, fog::distance);
    glUniform3fv(s_program.uniforms[su_FogColor].location, 1, fog::color.data());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, block_atlas::texture);
    glUniform1i(s_program.uniforms[su_AtlasTexture].location, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, block_atlas::tbo_frames);
    glUniform1i(s_program.uniforms[su_AtlasFrames].location, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_BUFFER, block_atlas::tbo_strips);
    glUniform1i(s_program.uniforms[su_AtlasStrips].location, 2);

    if(s_sorted_opaque.size()) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDisable(GL_BLEND);

        glBindVertexArray(s_vao);

        for(const auto& it : s_sorted_opaque) {
            const auto& mesh = world::chunk_entities.get<ChunkMesh>(it.first);
            const auto& chunk = world::chunk_entities.get<Chunk_Component>(it.first);

            if(frustum.intersects(utils::bounds(chunk.position - camera::chunk))) {
                draw_part(chunk, mesh.opaque);

                globals::num_draw_calls += 1;
                globals::num_draw_vertices += mesh.opaque.count * 6;
            }
        }
    }

    if(s_sorted_alpha.size()) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(s_vao);

        for(const auto& it : s_sorted_alpha) {
            const auto& mesh = world::chunk_entities.get<ChunkMesh>(it.first);
            const auto& chunk = world::chunk_entities.get<Chunk_Component>(it.first);

            if(frustum.intersects(utils::bounds(chunk.position - camera::chunk))) {
                draw_part(chunk, mesh.alpha);

                globals::num_draw_calls += 1;
                globals::num_draw_vertices += mesh.alpha.count * 6;
            }
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    if(s_sorted_fluid.size()) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(s_vao);

        for(const auto& it : s_sorted_fluid) {
            const auto& mesh = world::chunk_entities.get<ChunkMesh>(it.first);
            const auto& chunk = world::chunk_entities.get<Chunk_Component>(it.first);

            if(frustum.intersects(utils::bounds(chunk.position - camera::chunk))) {
                draw_part(chunk, mesh.fluid);

                globals::num_draw_calls += 1;
                globals::num_draw_vertices += mesh.fluid.count * 6;
            }
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}
