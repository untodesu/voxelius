#include "core/pch.hh"

#include "core/resource/image.hh"

#include "core/resource/resource.hh"

#include "core/utils/physfs.hh"

static int stbi_physfs_read(void* context, char* data, int size)
{
    return PHYSFS_readBytes(reinterpret_cast<PHYSFS_File*>(context), data, size);
}

static void stbi_physfs_skip(void* context, int count)
{
    auto file = reinterpret_cast<PHYSFS_File*>(context);
    PHYSFS_seek(file, PHYSFS_tell(file) + count);
}

static int stbi_physfs_eof(void* context)
{
    return PHYSFS_eof(reinterpret_cast<PHYSFS_File*>(context));
}

static const void* image_load_func(const char* name, std::uint32_t flags)
{
    assert(name);

    stbi_io_callbacks callbacks;
    callbacks.read = &stbi_physfs_read;
    callbacks.skip = &stbi_physfs_skip;
    callbacks.eof = &stbi_physfs_eof;

    stbi_set_flip_vertically_on_load(bool(flags & IMAGE_LOAD_FLIP));

    auto file = PHYSFS_openRead(name);

    if(file == nullptr) {
        spdlog::error("image: {}: {}", name, utils::physfs_error());
        return nullptr;
    }

    int desired_channels;

    if(flags & IMAGE_LOAD_GRAY) {
        desired_channels = STBI_grey;
    }
    else {
        desired_channels = STBI_rgb_alpha;
    }

    int width, height, channels;
    auto pixels = stbi_load_from_callbacks(&callbacks, file, &width, &height, &channels, desired_channels);

    PHYSFS_close(file);

    if(pixels == nullptr) {
        spdlog::error("image: {}: {}", name, stbi_failure_reason());
        return nullptr;
    }

    auto image = new Image;
    image->pixels = pixels;
    image->size = glm::ivec2(width, height);
    return image;
}

static void image_free_func(const void* resource)
{
    assert(resource);

    auto image = reinterpret_cast<const Image*>(resource);
    stbi_image_free(image->pixels);

    delete image;
}

void Image::register_resource(void)
{
    resource::register_loader<Image>(&image_load_func, &image_free_func);
}
