// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2024, Voxelius Contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
#include <client/image.hh>
#include <stb_image.h>

Image::~Image()
{
    unload();
}

bool Image::load_grayscale(const vfs::path_t &path, bool flip)
{
    stbi_set_flip_vertically_on_load(flip);

    unload();

    std::vector<uint8_t> buffer = {};
    if(vfs::read_bytes(path, buffer)) {
        const auto *buffer_p = reinterpret_cast<const stbi_uc *>(buffer.data());
        pixels = stbi_load_from_memory(buffer_p, static_cast<int>(buffer.size()), &width, &height, nullptr, STBI_grey);
        return width && height && pixels;
    }

    return false;
}

bool Image::load_rgba(const vfs::path_t &path, bool flip)
{
    stbi_set_flip_vertically_on_load(flip);

    unload();

    std::vector<uint8_t> buffer = {};
    if(vfs::read_bytes(path, buffer)) {
        const auto *buffer_p = reinterpret_cast<const stbi_uc *>(buffer.data());
        pixels = stbi_load_from_memory(buffer_p, static_cast<int>(buffer.size()), &width, &height, nullptr, STBI_rgb_alpha);
        return width && height && pixels;
    }

    return false;
}

void Image::unload()
{
    if(pixels)
        stbi_image_free(pixels);
    width = 0;
    height = 0;
    pixels = nullptr;
}

bool Image::valid() const
{
    return width && height && pixels;
}

int Image::get_width() const
{
    return width;
}

int Image::get_height() const
{
    return height;
}

const void *Image::data() const
{
    return pixels;
}

void *Image::data()
{
    return pixels;
}
