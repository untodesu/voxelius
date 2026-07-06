#include "client/pch.hh"

#include "client/res/texture2D.hh"

#include "core/res/image.hh"
#include "core/res/resource.hh"

static const void* texture2D_load_fn(const char* path, std::uint32_t flags)
{
    assert(path);

    std::uint32_t image_flags = 0;

    if(flags & RESFLAG_TEX_FLIP) {
        image_flags |= RESFLAG_IMG_FLIP;
    }

    if(flags & RESFLAG_TEX_GRAY) {
        image_flags |= RESFLAG_IMG_GRAY;
    }

    auto image = res::load_by_path<Image>(path, image_flags);

    if(image == nullptr) {
        LOG_WARNING("{}: failed to load image", path);
        return nullptr;
    }

    SDL_GPUTextureFormat format;

    if(flags & RESFLAG_TEX_GRAY) {
        format = SDL_GPU_TEXTUREFORMAT_R8_UNORM;
    }
    else {
        format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    }

    auto gpu = gpu::Texture::create_2D(image->width, image->height, format, SDL_GPU_TEXTUREUSAGE_SAMPLER);

    if(!gpu.is_valid()) {
        LOG_WARNING("{}: failed to create GPU texture", path);
        return nullptr;
    }

    gpu.write_blocking(std::as_bytes(std::span(image->pixels, static_cast<std::size_t>(image->width * image->height * image->channels))));

    auto texture = new Texture2D;
    texture->gpu = std::move(gpu);
    texture->imgui = reinterpret_cast<ImTextureID>(texture->gpu.get());

    return texture;
}

static void texture2D_free_fn(const void* resource)
{
    delete reinterpret_cast<const Texture2D*>(resource);
}

void Texture2D::register_resource(void)
{
    res::register_loader<Texture2D>(&texture2D_load_fn, &texture2D_free_fn);
}
