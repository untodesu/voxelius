#include "client/pch.hh"

#include "client/block_atlas.hh"

#include "core/exception.hh"
#include "core/res/image.hh"
#include "core/res/resource.hh"
#include "core/utils/crc64.hh"

#include "shared/block_registry.hh"
#include "shared/res/block_model.hh"

#include "client/globals.hh"

constexpr static int LAYER_SIZE = 4096;
constexpr static int LAYER_CAP = 256;
constexpr static int ATLAS_PADDING = 1;

GLuint block_atlas::texture;
GLuint block_atlas::vbo_frames;
GLuint block_atlas::tbo_frames;
GLuint block_atlas::vbo_strips;
GLuint block_atlas::tbo_strips;

static bool s_compiled = false;
static std::deque<AtlasStrip> s_strips;
static emhash8::HashMap<std::uint64_t, AtlasStrip*> s_lookup;
static std::vector<res::handle<Image>> s_pending;

static std::uint64_t span_hash(std::span<const Identifier> textures)
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

    glGenTextures(1, &block_atlas::texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, block_atlas::texture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, LAYER_SIZE, LAYER_SIZE, num_layers, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::vector<AtlasFrame_GL> gl_frames;
    gl_frames.reserve(s_pending.size());

    for(std::size_t i = 0; i < rects.size(); ++i) {
        const auto& rect = rects[i];
        const auto& image = s_pending.at(rect.id);

        AtlasFrame_GL frame {};
        frame.layer = layers[rect.id];
        frame.uv_min.x() = static_cast<float>(rect.x + ATLAS_PADDING) / static_cast<float>(LAYER_SIZE);
        frame.uv_min.y() = static_cast<float>(rect.y + ATLAS_PADDING) / static_cast<float>(LAYER_SIZE);
        frame.uv_max.x() = static_cast<float>(rect.x + ATLAS_PADDING + image->width) / static_cast<float>(LAYER_SIZE);
        frame.uv_max.y() = static_cast<float>(rect.y + ATLAS_PADDING + image->height) / static_cast<float>(LAYER_SIZE);

        gl_frames.emplace_back(std::move(frame));
    }

    glGenBuffers(1, &block_atlas::vbo_frames);
    glBindBuffer(GL_TEXTURE_BUFFER, block_atlas::vbo_frames);
    glBufferData(GL_TEXTURE_BUFFER, gl_frames.size() * sizeof(AtlasFrame_GL), gl_frames.data(), GL_STATIC_DRAW);

    glGenTextures(1, &block_atlas::tbo_frames);
    glBindTexture(GL_TEXTURE_BUFFER, block_atlas::tbo_frames);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, block_atlas::vbo_frames);

    std::vector<AtlasStrip_GL> gl_strips;
    gl_strips.reserve(s_strips.size());

    for(const auto& strip : s_strips) {
        AtlasStrip_GL gl_strip {};
        gl_strip.frame_base = static_cast<std::uint32_t>(strip.frame_base);
        gl_strip.frame_count = static_cast<std::uint32_t>(strip.frame_count);
        gl_strips.emplace_back(std::move(gl_strip));
    }

    glGenBuffers(1, &block_atlas::vbo_strips);
    glBindBuffer(GL_TEXTURE_BUFFER, block_atlas::vbo_strips);
    glBufferData(GL_TEXTURE_BUFFER, gl_strips.size() * sizeof(AtlasStrip_GL), gl_strips.data(), GL_STATIC_DRAW);

    glGenTextures(1, &block_atlas::tbo_strips);
    glBindTexture(GL_TEXTURE_BUFFER, block_atlas::tbo_strips);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32UI, block_atlas::vbo_strips);

    glBindTexture(GL_TEXTURE_2D_ARRAY, block_atlas::texture);

    for(const auto& rect : rects) {
        const auto& image = s_pending.at(rect.id);

        auto padded = extrude(image, ATLAS_PADDING);
        auto padded_wide = static_cast<GLsizei>(image->width + ATLAS_PADDING + ATLAS_PADDING);
        auto padded_tall = static_cast<GLsizei>(image->height + ATLAS_PADDING + ATLAS_PADDING);
        auto layer = static_cast<GLint>(layers.at(rect.id));

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, static_cast<GLint>(rect.x), static_cast<GLint>(rect.y), layer, padded_wide, padded_tall, 1,
            GL_RGBA, GL_UNSIGNED_BYTE, padded.data());
    }

    LOG_INFO("packed {} frames into {} layer(s) of a {}x{} atlas", rects.size(), num_layers, LAYER_SIZE, LAYER_SIZE);

    s_pending.clear();
    s_pending.shrink_to_fit();

    s_compiled = true;
}

const AtlasStrip* block_atlas::load(std::span<const Identifier> textures)
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
        strip.index = s_strips.size();

        s_strips.emplace_back(std::move(strip));

        auto strip_ptr = &s_strips.back();
        s_lookup.try_emplace(hash, strip_ptr);
        return strip_ptr;
    }

    return it->second;
}

const AtlasStrip* block_atlas::find(std::span<const Identifier> textures)
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
    texture = 0;
    tbo_frames = 0;
    vbo_frames = 0;
    tbo_strips = 0;
    vbo_strips = 0;

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
    glDeleteBuffers(1, &vbo_strips);
    glDeleteTextures(1, &tbo_strips);
    glDeleteBuffers(1, &vbo_frames);
    glDeleteTextures(1, &tbo_frames);
    glDeleteTextures(1, &texture);

    s_strips.clear();
    s_lookup.clear();
    s_pending.clear();

    s_compiled = false;
}
