#include "client/pch.hh"

#include "client/block_atlas.hh"

#include <stb_rect_pack.h>

#include "client/gpu/buffer.hh"
#include "client/gpu/texture.hh"

#include "core/res/image.hh"
#include "core/res/resource.hh"

#include "core/utils/crc64.hh"

#include "core/exception.hh"

#include "shared/res/block_model.hh"

#include "shared/block_registry.hh"

#include "client/globals.hh"

constexpr static int LAYER_SIZE = 4096;
constexpr static int LAYER_CAP = 256;
constexpr static int ATLAS_PADDING = 1;

std::unique_ptr<gpu::Texture> block_atlas::texture;
std::unique_ptr<gpu::Buffer> block_atlas::gpu_frames;
std::vector<AtlasFrame> block_atlas::cpu_frames;

static bool s_compiled = false;
static std::deque<AtlasStrip> s_strips;
static emhash8::HashMap<std::uint64_t, AtlasStrip*> s_lookup;
static std::vector<res::handle<Image>> s_pending;

static std::uint64_t span_hash(std::span<const Identifier> textures) noexcept
{
    std::uint64_t hash = 0;

    for(const auto& id : textures) {
        hash = utils::crc64(std::span(id.full_string()), hash);
    }

    return hash;
}

static std::vector<std::byte> extrude(const res::handle<Image>& image, int padding)
{
    auto pw = static_cast<std::size_t>(padding * 2 + image->width);
    auto ph = static_cast<std::size_t>(padding * 2 + image->height);

    std::vector<std::byte> pixels;
    pixels.resize(pw * ph * 4);

    for(int y = 0; y < ph; ++y) {
        auto sy = static_cast<std::size_t>(std::clamp(y - padding, 0, image->height - 1));

        for(int x = 0; x < pw; ++x) {
            auto sx = static_cast<std::size_t>(std::clamp(x - padding, 0, image->width - 1));

            auto src = reinterpret_cast<const std::byte*>(image->pixels);
            src += 4 * sy * static_cast<std::size_t>(image->width);
            src += 4 * sx;

            auto dst = pixels.data();
            dst += 4 * y * pw;
            dst += 4 * x;

            std::memcpy(dst, src, 4);
        }
    }

    return pixels;
}

static std::uint32_t pack_layers(std::vector<stbrp_rect>& rects, std::vector<std::uint32_t>& layers)
{
    layers.resize(rects.size());

    std::vector<std::size_t> remaining;
    remaining.resize(rects.size());

    for(std::size_t i = 0; i < rects.size(); ++i) {
        remaining[i] = i;
    }

    std::uint32_t num_layers = 0;
    std::vector<stbrp_node> nodes;
    nodes.resize(LAYER_SIZE);

    while(remaining.size()) {
        if(num_layers >= LAYER_CAP) {
            throw vx::runtime_error("block_atlas: atlas overflow");
        }

        std::vector<stbrp_rect> batch;
        batch.resize(remaining.size());

        for(std::size_t i = 0; i < remaining.size(); ++i) {
            batch[i] = rects[remaining[i]];
        }

        stbrp_context ctx;
        stbrp_init_target(&ctx, LAYER_SIZE, LAYER_SIZE, nodes.data(), static_cast<int>(nodes.size()));
        stbrp_pack_rects(&ctx, batch.data(), static_cast<int>(batch.size()));

        std::vector<std::size_t> next_remaining;
        next_remaining.reserve(remaining.size());

        for(std::size_t i = 0; i < batch.size(); ++i) {
            auto rect_index = remaining[i];

            if(batch[i].was_packed) {
                rects[rect_index].x = batch[i].x;
                rects[rect_index].y = batch[i].y;
                layers[rect_index] = num_layers;
            }
            else {
                next_remaining.push_back(rect_index);
            }
        }

        remaining = std::move(next_remaining);
        num_layers += 1;
    }

    return num_layers;
}

static void build_atlas(void)
{
    if(s_pending.empty()) {
        s_compiled = true;
        return;
    }

    std::vector<stbrp_rect> rects;
    rects.resize(s_pending.size());

    for(std::size_t i = 0; i < s_pending.size(); ++i) {
        const auto& image = s_pending[i];

        stbrp_rect rect {};
        rect.id = static_cast<int>(i);
        rect.w = static_cast<stbrp_coord>(image->width + ATLAS_PADDING * 2);
        rect.h = static_cast<stbrp_coord>(image->height + ATLAS_PADDING * 2);

        rects[i] = std::move(rect);
    }

    std::vector<std::uint32_t> layers;
    auto num_layers = pack_layers(rects, layers);

    auto texture = gpu::Texture::create_array(LAYER_SIZE, LAYER_SIZE, num_layers, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        SDL_GPU_TEXTUREUSAGE_SAMPLER, 1);
    vx::throw_if_not_fmt(texture.is_valid(), "block_atlas: failed to create a {}x{}x{} atlas texture", LAYER_SIZE, LAYER_SIZE, num_layers);

    block_atlas::texture = std::make_unique<gpu::Texture>(std::move(texture));
    block_atlas::cpu_frames.resize(s_pending.size());

    for(std::size_t i = 0; i < rects.size(); ++i) {
        const auto& rect = rects[i];
        const auto& image = s_pending.at(rect.id);

        AtlasFrame frame {};
        frame.layer = layers[rect.id];
        frame.uv_min.x() = static_cast<float>(rect.x + ATLAS_PADDING) / static_cast<float>(LAYER_SIZE);
        frame.uv_min.y() = static_cast<float>(rect.y + ATLAS_PADDING) / static_cast<float>(LAYER_SIZE);
        frame.uv_max.x() = static_cast<float>(rect.x + ATLAS_PADDING + image->width) / static_cast<float>(LAYER_SIZE);
        frame.uv_max.y() = static_cast<float>(rect.y + ATLAS_PADDING + image->height) / static_cast<float>(LAYER_SIZE);

        block_atlas::cpu_frames[rect.id] = std::move(frame);
    }

    auto frame_bytes = std::as_bytes(std::span(block_atlas::cpu_frames));
    auto frame_buffer = gpu::Buffer::create(frame_bytes.size(), SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ);
    vx::throw_if_not_fmt(frame_buffer.is_valid(), "block_atlas: failed to create a {} byte frame table buffer", frame_bytes.size());

    block_atlas::gpu_frames = std::make_unique<gpu::Buffer>(std::move(frame_buffer));

    auto cmds = SDL_AcquireGPUCommandBuffer(globals::gpu_device);
    vx::throw_if_fmt(cmds == nullptr, "SDL_AcquireGPUCommandBuffer failed: {}", SDL_GetError());

    auto pass = SDL_BeginGPUCopyPass(cmds);
    vx::throw_if(pass == nullptr, "SDL_BeginGPUCopyPass returned null!");

    for(const auto& rect : rects) {
        const auto& image = s_pending.at(rect.id);

        auto padded = extrude(image, ATLAS_PADDING);
        auto pw = static_cast<Uint32>(image->width + ATLAS_PADDING * 2);
        auto ph = static_cast<Uint32>(image->height + ATLAS_PADDING * 2);
        auto layer = layers[rect.id];

        block_atlas::texture->write_streamed(pass, padded, layer, 0, static_cast<Uint32>(rect.x), static_cast<Uint32>(rect.y), pw, ph);
    }

    block_atlas::gpu_frames->write_streamed(pass, frame_bytes);

    SDL_EndGPUCopyPass(pass);

    auto fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmds);
    SDL_WaitForGPUFences(globals::gpu_device, true, &fence, 1);
    SDL_ReleaseGPUFence(globals::gpu_device, fence);

    LOG_INFO("packed {} frames into {} layer(s) of a {}x{} atlas", rects.size(), num_layers, LAYER_SIZE, LAYER_SIZE);

    s_pending.clear();
    s_pending.shrink_to_fit();

    s_compiled = true;
}

const AtlasStrip* block_atlas::load(std::span<const Identifier> textures) noexcept
{
    if(textures.empty() || s_compiled) {
        return nullptr;
    }

    auto hash = span_hash(textures);
    auto it = s_lookup.find(hash);

    if(it == s_lookup.cend()) {
        std::size_t frame_base = s_pending.size();
        std::size_t frame_count = 0;

        for(const auto& id : textures) {
            auto image = res::load<Image>(id, "textures/block");

            if(image == nullptr) {
                LOG_WARNING("{}: failed to load image", id.full_string());
                continue;
            }

            s_pending.push_back(std::move(image));
            frame_count += 1;
        }

        if(frame_count == 0) {
            return nullptr;
        }

        AtlasStrip strip {};
        strip.frame_base = frame_base;
        strip.frame_count = frame_count;

        s_strips.emplace_back(std::move(strip));

        auto strip_ptr = &s_strips.back();
        s_lookup.try_emplace(hash, strip_ptr);
        return strip_ptr;
    }

    return it->second;
}

const AtlasStrip* block_atlas::find(std::span<const Identifier> textures) noexcept
{
    if(textures.empty()) {
        return nullptr;
    }

    auto hash = span_hash(textures);
    auto it = s_lookup.find(hash);

    if(it == s_lookup.cend()) {
        return nullptr;
    }

    return it->second;
}

void block_atlas::init(void)
{
    texture.reset();
    gpu_frames.reset();
    cpu_frames.clear();

    s_strips.clear();
    s_lookup.clear();
    s_pending.clear();

    s_compiled = false;
}

void block_atlas::init_late(void)
{
    for(const auto& def : block_registry::all_definitions()) {
        if(def.is_stem || def.model_name.is_empty()) {
            continue;
        }

        auto model = res::load<BlockModel>(def.model_name, "models/block", ".json");

        if(model == nullptr) {
            LOG_WARNING("{}: failed to load block model", def.model_name.full_string());
            continue;
        }

        for(const auto& slot : model->texture_slots) {
            auto frames = def.resolve_texture_slot(slot);

            if(!frames.has_value()) {
                LOG_WARNING("{}: missing texture for slot '{}'", def.model_name.full_string(), slot);
                continue;
            }

            if(nullptr == block_atlas::load(frames.value())) {
                LOG_WARNING("{}: failed to load atlas strip for slot '{}'", def.model_name.full_string(), slot);
            }
        }
    }

    build_atlas();
}

void block_atlas::shutdown(void)
{
    texture.reset();
    gpu_frames.reset();
    cpu_frames.clear();

    s_strips.clear();
    s_lookup.clear();
    s_pending.clear();

    s_compiled = false;
}
