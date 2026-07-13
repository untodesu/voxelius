#include "client/pch.hh"

#include "client/chunk_renderer.hh"

#include "core/camera.hh"
#include "core/exception.hh"

#include "shared/chunk.hh"
#include "shared/constant.hh"
#include "shared/utils/chunk.hh"
#include "shared/utils/coord.hh"
#include "shared/world.hh"

#include "client/block_atlas.hh"
#include "client/camera.hh"
#include "client/chunk_mesh.hh"
#include "client/globals.hh"
#include "client/gpu/buffer.hh"
#include "client/gpu/sampler.hh"
#include "client/gpu/texture.hh"

constexpr static Uint32 VERTEX_COUNT = 6;

// Compiled from hlsl/chunk.vert.hlsl
extern const std::uint8_t spirv_chunk_vert[];
extern const std::size_t spirv_chunk_vert_size;

// Compiled from hlsl/chunk.frag.hlsl
extern const std::uint8_t spirv_chunk_frag[];
extern const std::size_t spirv_chunk_frag_size;

struct Uniforms_PerFrame final {
    alignas(16) Eigen::Matrix4f view_projection;
    alignas(4) std::uint32_t animation_timer;
};

struct Uniforms_PerChunk final {
    alignas(16) Eigen::Vector3f world_position;
};

static SDL_GPUGraphicsPipeline* s_pipeline_opaque;
static SDL_GPUGraphicsPipeline* s_pipeline_alpha;
static gpu::Sampler s_atlas_sampler;

static SDL_GPUGraphicsPipeline* create_pipeline(SDL_GPUShader* vert, SDL_GPUShader* frag, SDL_GPUTextureFormat swapchain_format, bool blend,
    bool depth_write, SDL_GPUCompareOp depth_compare, bool color_target_enabled)
{
    SDL_GPUVertexBufferDescription vertex_buffer_desc {};
    vertex_buffer_desc.slot = 0;
    vertex_buffer_desc.pitch = sizeof(ChunkMesh_Quad);
    vertex_buffer_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE;

    std::array<SDL_GPUVertexAttribute, 5> attributes {};
    attributes[0] = { 0, 0, SDL_GPU_VERTEXELEMENTFORMAT_UINT, static_cast<Uint32>(offsetof(ChunkMesh_Quad, data_origin)) };
    attributes[1] = { 1, 0, SDL_GPU_VERTEXELEMENTFORMAT_UINT, static_cast<Uint32>(offsetof(ChunkMesh_Quad, data_edge_u)) };
    attributes[2] = { 2, 0, SDL_GPU_VERTEXELEMENTFORMAT_UINT, static_cast<Uint32>(offsetof(ChunkMesh_Quad, data_edge_v)) };
    attributes[3] = { 3, 0, SDL_GPU_VERTEXELEMENTFORMAT_UINT, static_cast<Uint32>(offsetof(ChunkMesh_Quad, data_uv)) };
    attributes[4] = { 4, 0, SDL_GPU_VERTEXELEMENTFORMAT_UINT, static_cast<Uint32>(offsetof(ChunkMesh_Quad, data_texture)) };

    SDL_GPUColorTargetDescription color_target {};
    color_target.format = swapchain_format;

    if(blend) {
        color_target.blend_state.enable_blend = true;
        color_target.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
        color_target.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
        color_target.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        color_target.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        color_target.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        color_target.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    }

    SDL_GPUGraphicsPipelineCreateInfo info {};
    info.vertex_shader = vert;
    info.fragment_shader = frag;

    info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    info.vertex_input_state.vertex_buffer_descriptions = &vertex_buffer_desc;
    info.vertex_input_state.num_vertex_buffers = 1;
    info.vertex_input_state.vertex_attributes = attributes.data();
    info.vertex_input_state.num_vertex_attributes = static_cast<Uint32>(attributes.size());

    info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
    info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

    info.depth_stencil_state.enable_depth_test = true;
    info.depth_stencil_state.enable_depth_write = depth_write;
    info.depth_stencil_state.compare_op = depth_compare;

    info.target_info.num_color_targets = color_target_enabled ? 1 : 0;
    info.target_info.color_target_descriptions = &color_target;
    info.target_info.has_depth_stencil_target = true;
    info.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;

    return SDL_CreateGPUGraphicsPipeline(globals::gpu_device, &info);
}

static void upload_part(SDL_GPUCopyPass* copy_pass, ChunkMesh_Part& part)
{
    if(part.quads.empty()) {
        part.quad_buffer = gpu::Buffer();
        part.quad_count = 0;
        return;
    }

    auto bytes = std::as_bytes(std::span(part.quads));
    part.quad_buffer = gpu::Buffer::create(bytes.size(), SDL_GPU_BUFFERUSAGE_VERTEX);
    part.quad_buffer.write_streamed(copy_pass, bytes);
    part.quad_count = static_cast<std::uint32_t>(part.quads.size());

    // FIXME: might not want to clear these for blendable blocks
    part.quads.clear();
    part.quads.shrink_to_fit();
}

static void draw_part(SDL_GPURenderPass* render_pass, const Chunk_Component& chunk, const ChunkMesh_Part& part)
{
    Uniforms_PerChunk per_chunk {};
    per_chunk.world_position = utils::to_fvec(chunk.position - camera::chunk);
    SDL_PushGPUVertexUniformData(globals::gpu_commands_main, 1, &per_chunk, sizeof(per_chunk));

    SDL_GPUBufferBinding vbo_binding {};
    vbo_binding.buffer = part.quad_buffer.get();
    vbo_binding.offset = 0;

    SDL_BindGPUVertexBuffers(render_pass, 0, &vbo_binding, 1);
    SDL_DrawGPUPrimitives(render_pass, VERTEX_COUNT, part.quad_count, 0, 0);
}

static float chunk_distance_sq(entt::entity entity)
{
    const auto& chunk = world::chunk_entities.get<Chunk_Component>(entity);
    Eigen::Vector3f position = utils::to_fvec(chunk.position - camera::chunk) * static_cast<float>(constant::CHUNK_SIZE);
    Eigen::Vector3f delta = position - camera::local;
    return delta.squaredNorm();
}

void chunk_renderer::init(void)
{
    SDL_GPUShaderCreateInfo vert_info {};
    vert_info.code_size = spirv_chunk_vert_size;
    vert_info.code = spirv_chunk_vert;
    vert_info.entrypoint = "main";
    vert_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    vert_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
    vert_info.num_uniform_buffers = 2;
    vert_info.num_storage_buffers = 1;

    SDL_GPUShaderCreateInfo frag_info {};
    frag_info.code_size = spirv_chunk_frag_size;
    frag_info.code = spirv_chunk_frag;
    frag_info.entrypoint = "main";
    frag_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    frag_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    frag_info.num_samplers = 1;
    frag_info.num_storage_buffers = 1;

    auto vert = SDL_CreateGPUShader(globals::gpu_device, &vert_info);
    auto frag = SDL_CreateGPUShader(globals::gpu_device, &frag_info);

    vx::throw_if_not_fmt(vert, "chunk_renderer: failed to create vertex shader: {}", SDL_GetError());
    vx::throw_if_not_fmt(frag, "chunk_renderer: failed to create fragment shader: {}", SDL_GetError());

    auto swapchain_format = SDL_GetGPUSwapchainTextureFormat(globals::gpu_device, globals::window);
    s_pipeline_opaque = create_pipeline(vert, frag, swapchain_format, false, true, SDL_GPU_COMPAREOP_LESS_OR_EQUAL, true);
    s_pipeline_alpha = create_pipeline(vert, frag, swapchain_format, true, false, SDL_GPU_COMPAREOP_LESS_OR_EQUAL, true);

    SDL_ReleaseGPUShader(globals::gpu_device, frag);
    SDL_ReleaseGPUShader(globals::gpu_device, vert);

    vx::throw_if_not_fmt(s_pipeline_opaque, "chunk_renderer: failed to create the opaque pipeline: {}", SDL_GetError());
    vx::throw_if_not_fmt(s_pipeline_alpha, "chunk_renderer: failed to create the alpha pipeline: {}", SDL_GetError());

    s_atlas_sampler = gpu::Sampler::create(SDL_GPU_FILTER_NEAREST, SDL_GPU_SAMPLERMIPMAPMODE_NEAREST, SDL_GPU_SAMPLERADDRESSMODE_REPEAT);
    vx::throw_if_not_fmt(s_atlas_sampler.is_valid(), "chunk_renderer: failed to create the atlas sampler: {}", SDL_GetError());
}

void chunk_renderer::shutdown(void)
{
    s_atlas_sampler.reset();

    SDL_ReleaseGPUGraphicsPipeline(globals::gpu_device, s_pipeline_alpha);
    SDL_ReleaseGPUGraphicsPipeline(globals::gpu_device, s_pipeline_opaque);

    s_pipeline_alpha = nullptr;
    s_pipeline_opaque = nullptr;
}

void chunk_renderer::upload(SDL_GPUCopyPass* copy_pass)
{
    auto view = world::chunk_entities.view<ChunkMesh_UploadMarker, ChunkMesh>();

    for(auto [entity, mesh] : view.each()) {
        world::chunk_entities.remove<ChunkMesh_UploadMarker>(entity);
        upload_part(copy_pass, mesh.opaque);
        upload_part(copy_pass, mesh.alpha);
    }
}

void chunk_renderer::render(SDL_GPURenderPass* render_pass)
{
    static std::vector<entt::entity> opaque_chunks;
    static std::vector<entt::entity> alpha_chunks;

    if(block_atlas::texture == nullptr || block_atlas::gpu_frames == nullptr || block_atlas::gpu_strips == nullptr) {
        return;
    }

    std::array<SDL_GPUTextureSamplerBinding, 1> texture_binding {};
    texture_binding[0].texture = block_atlas::texture->get();
    texture_binding[0].sampler = s_atlas_sampler.get();

    auto storage_buffer = block_atlas::gpu_frames->get();
    auto strip_buffer = block_atlas::gpu_strips->get();
    auto view = world::chunk_entities.view<Chunk_Component, ChunkMesh>();

    Uniforms_PerFrame per_frame {};
    per_frame.view_projection = camera::instance.view_projection();
    per_frame.animation_timer = static_cast<std::uint32_t>(world::current_tick >> 1);

    const auto& frustum = camera::instance.frustum();

    opaque_chunks.clear();
    opaque_chunks.reserve(view.size_hint());

    for(const auto [entity, chunk, mesh] : view.each()) {
        auto visible = true;
        visible = visible && mesh.opaque.quad_buffer.is_valid();
        visible = visible && frustum.intersects(utils::bounds(chunk.position - camera::chunk));

        if(visible) {
            opaque_chunks.push_back(entity);
        }
    }

    std::sort(opaque_chunks.begin(), opaque_chunks.end(), [&](entt::entity a, entt::entity b) {
        auto dist_a = chunk_distance_sq(a);
        auto dist_b = chunk_distance_sq(b);
        return dist_a < dist_b;
    });

    SDL_BindGPUGraphicsPipeline(render_pass, s_pipeline_opaque);
    SDL_PushGPUVertexUniformData(globals::gpu_commands_main, 0, &per_frame, sizeof(per_frame));
    SDL_BindGPUVertexStorageBuffers(render_pass, 0, &strip_buffer, 1);
    SDL_BindGPUFragmentSamplers(render_pass, 0, texture_binding.data(), static_cast<Uint32>(texture_binding.size()));
    SDL_BindGPUFragmentStorageBuffers(render_pass, 0, &storage_buffer, 1);

    for(auto entity : opaque_chunks) {
        const auto& mesh = world::chunk_entities.get<ChunkMesh>(entity);

        draw_part(render_pass, world::chunk_entities.get<Chunk_Component>(entity), mesh.opaque);

        globals::num_draw_calls += 1;
        globals::num_draw_vertices += mesh.opaque.quad_count * VERTEX_COUNT;
    }

    alpha_chunks.clear();
    alpha_chunks.reserve(view.size_hint());

    for(const auto [entity, chunk, mesh] : view.each()) {
        auto visible = true;
        visible = visible && mesh.alpha.quad_buffer.is_valid();
        visible = visible && frustum.intersects(utils::bounds(chunk.position - camera::chunk));

        if(visible) {
            alpha_chunks.push_back(entity);
        }
    }

    if(alpha_chunks.empty()) {
        return;
    }

    std::sort(alpha_chunks.begin(), alpha_chunks.end(), [&](entt::entity a, entt::entity b) {
        auto a_dist = chunk_distance_sq(a);
        auto b_dist = chunk_distance_sq(b);
        return a_dist > b_dist;
    });

    SDL_BindGPUGraphicsPipeline(render_pass, s_pipeline_alpha);
    SDL_PushGPUVertexUniformData(globals::gpu_commands_main, 0, &per_frame, sizeof(per_frame));
    SDL_BindGPUVertexStorageBuffers(render_pass, 0, &strip_buffer, 1);
    SDL_BindGPUFragmentSamplers(render_pass, 0, texture_binding.data(), static_cast<Uint32>(texture_binding.size()));
    SDL_BindGPUFragmentStorageBuffers(render_pass, 0, &storage_buffer, 1);

    for(auto entity : alpha_chunks) {
        const auto& mesh = world::chunk_entities.get<ChunkMesh>(entity);

        draw_part(render_pass, world::chunk_entities.get<Chunk_Component>(entity), mesh.alpha);

        globals::num_draw_calls += 1;
        globals::num_draw_vertices += mesh.alpha.quad_count * VERTEX_COUNT;
    }
}
