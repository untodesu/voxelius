#include "client/pch.hh"

#include "client/resource/texture_gui.hh"

#include "core/resource/image.hh"
#include "core/resource/resource.hh"

static const void* texture_gui_load_func(const char* name, std::uint32_t flags)
{
    assert(name);

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
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        }

        if(flags & TEXTURE_GUI_LOAD_CLAMP_T) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        if(flags & TEXTURE_GUI_LOAD_LINEAR_MAG) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        if(flags & TEXTURE_GUI_LOAD_LINEAR_MIN) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        auto new_resource = new TextureGUI();
        new_resource->handle = static_cast<ImTextureID>(gl_texture);
        new_resource->size.x = image->size.x;
        new_resource->size.y = image->size.y;

        return new_resource;
    }

    return nullptr;
}

static void texture_gui_free_func(const void* resource)
{
    assert(resource);

    auto texture_gui = reinterpret_cast<const TextureGUI*>(resource);
    auto gl_texture = static_cast<GLuint>(texture_gui->handle);

    glDeleteTextures(1, &gl_texture);

    delete texture_gui;
}

void TextureGUI::register_resource(void)
{
    resource::register_loader<TextureGUI>(&texture_gui_load_func, &texture_gui_free_func);
}
