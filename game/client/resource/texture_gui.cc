#include "client/pch.hh"

#include "client/resource/texture_gui.hh"

#include "core/resource/image.hh"
#include "core/resource/resource.hh"

static emhash8::HashMap<std::string, resource_ptr<TextureGUI>> resource_map;

template<>
resource_ptr<TextureGUI> resource::load<TextureGUI>(const char* name, unsigned int flags)
{
    auto it = resource_map.find(name);

    if(it != resource_map.cend()) {
        // Return an existing resource
        return it->second;
    }

    unsigned int image_load_flags = 0U;

    if(flags & TEXTURE_GUI_LOAD_VFLIP) {
        image_load_flags |= IMAGE_LOAD_FLIP;
    }

    if(flags & TEXTURE_GUI_LOAD_GRAYSCALE) {
        image_load_flags |= IMAGE_LOAD_GRAY;
    }

    if(auto image = resource::load<Image>(name, image_load_flags)) {
        GLuint gl_texture;

        glGenTextures(1, &gl_texture);
        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->size.x, image->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);

        if(flags & TEXTURE_GUI_LOAD_CLAMP_S) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        }

        if(flags & TEXTURE_GUI_LOAD_CLAMP_T) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        if(flags & TEXTURE_GUI_LOAD_LINEAR_MAG) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        if(flags & TEXTURE_GUI_LOAD_LINEAR_MIN) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        auto new_resource = std::make_shared<TextureGUI>();
        new_resource->handle = static_cast<ImTextureID>(gl_texture);
        new_resource->size.x = image->size.x;
        new_resource->size.y = image->size.y;

        return resource_map.insert_or_assign(name, new_resource).first->second;
    }

    return nullptr;
}

template<>
void resource::hard_cleanup<TextureGUI>(void)
{
    for(const auto& it : resource_map) {
        if(it.second.use_count() > 1L) {
            spdlog::warn("resource: zombie resource [TextureGUI] {} [use_count={}]", it.first, it.second.use_count());
        } else {
            spdlog::debug("resource: releasing [TextureGUI] {}", it.first);
        }

        auto gl_texture = static_cast<GLuint>(it.second->handle);

        glDeleteTextures(1, &gl_texture);
    }

    resource_map.clear();
}

template<>
void resource::soft_cleanup<TextureGUI>(void)
{
    auto iter = resource_map.cbegin();

    while(iter != resource_map.cend()) {
        if(iter->second.use_count() == 1L) {
            spdlog::debug("resource: releasing [TextureGUI] {}", iter->first);

            auto gl_texture = static_cast<GLuint>(iter->second->handle);

            glDeleteTextures(1, &gl_texture);

            iter = resource_map.erase(iter);

            continue;
        }

        iter = std::next(iter);
    }
}
