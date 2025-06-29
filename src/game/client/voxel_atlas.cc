#include "client/pch.hh"

#include "client/voxel_atlas.hh"

#include "core/constexpr.hh"
#include "core/crc64.hh"
#include "core/image.hh"
#include "core/resource.hh"

struct AtlasPlane final {
    std::unordered_map<std::size_t, std::size_t> lookup;
    std::vector<AtlasStrip> strips;
    std::size_t layer_count_max;
    std::size_t layer_count;
    std::size_t plane_id;
    GLuint gl_texture;
};

static int atlas_width;
static int atlas_height;
static std::size_t atlas_count;
static std::vector<AtlasPlane> planes;

// Certain animated and varied voxels just double their
// textures (see the "default" texture part in VoxelInfoBuilder::build)
// so there could either be six UNIQUE atlas strips or only one
// https://crypto.stackexchange.com/questions/55162/best-way-to-hash-two-values-into-one
static std::size_t vector_hash(const std::vector<std::string>& strings)
{
    std::size_t source = 0;
    for(const std::string& str : strings)
        source += crc64::get(str);
    return crc64::get(&source, sizeof(source));
}

static void plane_setup(AtlasPlane& plane)
{
    glGenTextures(1, &plane.gl_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, plane.gl_texture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, atlas_width, atlas_height, plane.layer_count_max, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

static AtlasStrip* plane_lookup(AtlasPlane& plane, std::size_t hash_value)
{
    const auto it = plane.lookup.find(hash_value);

    if(it != plane.lookup.cend()) {
        return &plane.strips[it->second];
    }

    return nullptr;
}

static AtlasStrip* plane_new_strip(AtlasPlane& plane, const std::vector<std::string>& paths, std::size_t hash_value)
{
    AtlasStrip strip = {};
    strip.offset = plane.layer_count;
    strip.plane = plane.plane_id;

    glBindTexture(GL_TEXTURE_2D_ARRAY, plane.gl_texture);

    for(std::size_t i = 0; i < paths.size(); ++i) {
        if(auto image = resource::load<Image>(paths[i].c_str(), IMAGE_LOAD_FLIP)) {
            if((image->size.x != atlas_width) || (image->size.y != atlas_height)) {
                spdlog::warn("atlas: {}: size mismatch", paths[i]);
                continue;
            }

            const std::size_t offset = strip.offset + i;
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY, 0, 0, 0, offset, image->size.x, image->size.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
        }
    }

    plane.layer_count += paths.size();

    const std::size_t index = plane.strips.size();
    plane.lookup.emplace(hash_value, index);
    plane.strips.push_back(std::move(strip));
    return &plane.strips[index];
}

void voxel_atlas::create(int width, int height, std::size_t count)
{
    GLint max_plane_layers;

    atlas_width = 1 << vx::log2(width);
    atlas_height = 1 << vx::log2(height);

    // Clipping this at OpenGL 4.5 limit of 2048 is important due to
    // how voxel quad meshes are packed in memory: each texture index is
    // confined in 11 bits so having bigger atlas planes makes no sense;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_plane_layers);
    max_plane_layers = vx::clamp(max_plane_layers, 256, 2048);

    for(long i = count; i > 0L; i -= max_plane_layers) {
        AtlasPlane plane = {};
        plane.plane_id = planes.size();
        plane.layer_count_max = vx::min<std::size_t>(max_plane_layers, i);
        plane.layer_count = 0;

        const std::size_t save_id = plane.plane_id;
        planes.push_back(std::move(plane));
        plane_setup(planes[save_id]);
    }

    spdlog::debug("voxel_atlas: count={}", count);
    spdlog::debug("voxel_atlas: atlas_size=[{}x{}]", atlas_width, atlas_height);
    spdlog::debug("voxel_atlas: max_plane_layers={}", max_plane_layers);
}

void voxel_atlas::destroy(void)
{
    for(const AtlasPlane& plane : planes)
        glDeleteTextures(1, &plane.gl_texture);
    atlas_width = 0;
    atlas_height = 0;
    planes.clear();
}

std::size_t voxel_atlas::plane_count(void)
{
    return planes.size();
}

GLuint voxel_atlas::plane_texture(std::size_t plane_id)
{
    if(plane_id < planes.size()) {
        return planes[plane_id].gl_texture;
    } else {
        return 0;
    }
}

void voxel_atlas::generate_mipmaps(void)
{
    for(const AtlasPlane& plane : planes) {
        glBindTexture(GL_TEXTURE_2D_ARRAY, plane.gl_texture);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }
}

AtlasStrip* voxel_atlas::find_or_load(const std::vector<std::string>& paths)
{
    const std::size_t hash_value = vector_hash(paths);

    for(AtlasPlane& plane : planes) {
        if(AtlasStrip* strip = plane_lookup(plane, hash_value)) {
            return strip;
        }

        continue;
    }

    for(AtlasPlane& plane : planes) {
        if((plane.layer_count + paths.size()) <= plane.layer_count_max) {
            return plane_new_strip(plane, paths, hash_value);
        }

        continue;
    }

    return nullptr;
}

AtlasStrip* voxel_atlas::find(const std::vector<std::string>& paths)
{
    const std::size_t hash_value = vector_hash(paths);

    for(AtlasPlane& plane : planes) {
        if(AtlasStrip* strip = plane_lookup(plane, hash_value)) {
            return strip;
        }

        continue;
    }

    return nullptr;
}
