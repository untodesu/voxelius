#include "client/pch.hh"

#include "client/chunk_renderer.hh"

#include "core/camera.hh"
#include "core/exception.hh"
#include "core/threading.hh"

#include "shared/chunk.hh"
#include "shared/constant.hh"
#include "shared/utils/chunk.hh"
#include "shared/utils/coord.hh"
#include "shared/world.hh"

#include "client/block_atlas.hh"
#include "client/camera.hh"
#include "client/chunk_mesh.hh"
#include "client/globals.hh"
#include "client/shader_program.hh"

static ShaderProgram s_opaque;
static std::size_t s_opaque_ViewProjection;
static std::size_t s_opaque_AnimationTimer;
static std::size_t s_opaque_WorldPosition;
static std::size_t s_opaque_AtlasTexture;
static std::size_t s_opaque_AtlasStrips;
static std::size_t s_opaque_AtlasFrames;

static ShaderProgram s_alpha;
static std::size_t s_alpha_ViewProjection;
static std::size_t s_alpha_AnimationTimer;
static std::size_t s_alpha_WorldPosition;
static std::size_t s_alpha_AtlasTexture;
static std::size_t s_alpha_AtlasStrips;
static std::size_t s_alpha_AtlasFrames;

static GLuint s_vao;

static bool s_sorted_dirty;
static std::vector<std::pair<entt::entity, float>> s_sorted_opaque;
static std::vector<std::pair<entt::entity, float>> s_sorted_alpha;

static chunk_pos s_last_camera_chunk;

static void draw_part(const Chunk_Component& chunk, const ChunkMesh_Part& part, GLint u_WorldPosition)
{
    if(part.count == 0 || part.vbo == 0) {
        return;
    }

    glUniform3fv(u_WorldPosition, 1, utils::to_fvec(chunk.position - camera::chunk).data());

    glBindBuffer(GL_ARRAY_BUFFER, part.vbo);

    auto stride = static_cast<GLsizei>(sizeof(ChunkMesh_Quad));
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_origin)));
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_edge_u)));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_edge_v)));
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_uv)));
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, stride, (const void*)(offsetof(ChunkMesh_Quad, data_texture)));

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
    auto opaque_id = Identifier::from_parts(constant::BUILTIN_NAME_SPACE, "chunk");
    auto opaque_ok = s_opaque.setup(opaque_id);
    vx::throw_if_not_fmt(opaque_ok, "{}: setup failed", opaque_id.full_string());

    s_opaque_ViewProjection = s_opaque.add_uniform("u_ViewProjection");
    s_opaque_AnimationTimer = s_opaque.add_uniform("u_AnimationTimer");
    s_opaque_WorldPosition = s_opaque.add_uniform("u_WorldPosition");
    s_opaque_AtlasTexture = s_opaque.add_uniform("u_AtlasTexture");
    s_opaque_AtlasStrips = s_opaque.add_uniform("u_AtlasStrips");
    s_opaque_AtlasFrames = s_opaque.add_uniform("u_AtlasFrames");

    auto alpha_id = Identifier::from_parts(constant::BUILTIN_NAME_SPACE, "chunk");
    auto alpha_ok = s_alpha.setup(alpha_id);
    vx::throw_if_not_fmt(alpha_ok, "{}: setup failed", alpha_id.full_string());

    s_alpha_ViewProjection = s_alpha.add_uniform("u_ViewProjection");
    s_alpha_AnimationTimer = s_alpha.add_uniform("u_AnimationTimer");
    s_alpha_WorldPosition = s_alpha.add_uniform("u_WorldPosition");
    s_alpha_AtlasTexture = s_alpha.add_uniform("u_AtlasTexture");
    s_alpha_AtlasStrips = s_alpha.add_uniform("u_AtlasStrips");
    s_alpha_AtlasFrames = s_alpha.add_uniform("u_AtlasFrames");

    glGenVertexArrays(1, &s_vao);
    glBindVertexArray(s_vao);

    for(GLuint i = 0; i < 5; ++i) {
        glEnableVertexAttribArray(i);
        glVertexAttribDivisor(i, 1);
    }

    s_sorted_dirty = true;
    s_sorted_opaque.clear();
    s_sorted_alpha.clear();

    s_last_camera_chunk = camera::chunk + chunk_pos::Ones();

    globals::dispatcher.sink<ChunkCreateEvent>().connect<&on_chunk_create>();
    globals::dispatcher.sink<ChunkRemoveEvent>().connect<&on_chunk_remove>();
    globals::dispatcher.sink<ChunkUpdateEvent>().connect<&on_chunk_update>();
    world::chunk_entities.on_update<ChunkMesh>().connect<&on_chunk_mesh>();
}

void chunk_renderer::shutdown(void)
{
    glDeleteVertexArrays(1, &s_vao);

    s_alpha_AtlasFrames = std::numeric_limits<std::size_t>::max();
    s_alpha_AtlasStrips = std::numeric_limits<std::size_t>::max();
    s_alpha_AtlasTexture = std::numeric_limits<std::size_t>::max();
    s_alpha_WorldPosition = std::numeric_limits<std::size_t>::max();
    s_alpha_AnimationTimer = std::numeric_limits<std::size_t>::max();
    s_alpha_ViewProjection = std::numeric_limits<std::size_t>::max();
    s_alpha.destroy();

    s_opaque_AtlasFrames = std::numeric_limits<std::size_t>::max();
    s_opaque_AtlasStrips = std::numeric_limits<std::size_t>::max();
    s_opaque_AtlasTexture = std::numeric_limits<std::size_t>::max();
    s_opaque_WorldPosition = std::numeric_limits<std::size_t>::max();
    s_opaque_AnimationTimer = std::numeric_limits<std::size_t>::max();
    s_opaque_ViewProjection = std::numeric_limits<std::size_t>::max();
    s_opaque.destroy();
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

    vx::throw_if_not(s_opaque.update());
    vx::throw_if_not(s_alpha.update());

    const auto& vproj = camera::instance.view_projection();
    const auto& frustum = camera::instance.frustum();

    auto animation_timer = static_cast<std::uint32_t>(world::current_tick >> 2);

    if(s_sorted_opaque.size()) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glDisable(GL_BLEND);

        glUseProgram(s_opaque.handle);
        glUniformMatrix4fv(s_opaque.uniforms[s_opaque_ViewProjection].location, 1, GL_FALSE, vproj.data());
        glUniform1ui(s_opaque.uniforms[s_opaque_AnimationTimer].location, animation_timer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, block_atlas::texture);
        glUniform1i(s_opaque.uniforms[s_opaque_AtlasTexture].location, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, block_atlas::tbo_frames);
        glUniform1i(s_opaque.uniforms[s_opaque_AtlasFrames].location, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_BUFFER, block_atlas::tbo_strips);
        glUniform1i(s_opaque.uniforms[s_opaque_AtlasStrips].location, 2);

        glBindVertexArray(s_vao);

        for(const auto& it : s_sorted_opaque) {
            const auto& mesh = world::chunk_entities.get<ChunkMesh>(it.first);
            const auto& chunk = world::chunk_entities.get<Chunk_Component>(it.first);

            if(frustum.intersects(utils::bounds(chunk.position - camera::chunk))) {
                draw_part(chunk, mesh.opaque, s_opaque.uniforms[s_opaque_WorldPosition].location);

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

        glUseProgram(s_alpha.handle);
        glUniformMatrix4fv(s_alpha.uniforms[s_alpha_ViewProjection].location, 1, GL_FALSE, vproj.data());
        glUniform1ui(s_alpha.uniforms[s_alpha_AnimationTimer].location, animation_timer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, block_atlas::texture);
        glUniform1i(s_alpha.uniforms[s_alpha_AtlasTexture].location, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, block_atlas::tbo_frames);
        glUniform1i(s_alpha.uniforms[s_alpha_AtlasFrames].location, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_BUFFER, block_atlas::tbo_strips);
        glUniform1i(s_alpha.uniforms[s_alpha_AtlasStrips].location, 2);

        glBindVertexArray(s_vao);

        for(const auto& it : s_sorted_opaque) {
            const auto& mesh = world::chunk_entities.get<ChunkMesh>(it.first);
            const auto& chunk = world::chunk_entities.get<Chunk_Component>(it.first);

            if(frustum.intersects(utils::bounds(chunk.position - camera::chunk))) {
                draw_part(chunk, mesh.alpha, s_alpha.uniforms[s_alpha_WorldPosition].location);

                globals::num_draw_calls += 1;
                globals::num_draw_vertices += mesh.alpha.count * 6;
            }
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}
