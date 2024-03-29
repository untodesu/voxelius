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
#ifndef CLIENT_GLXX_SAMPLER_HH
#define CLIENT_GLXX_SAMPLER_HH
#include <client/glxx/object.hh>

namespace glxx
{
class Sampler final : public glxx::Object<Sampler> {
public:
    Sampler() = default;
    Sampler(Sampler &&rhs);
    Sampler &operator=(Sampler &&rhs);
    void create();
    void destroy();
    void bind(uint32_t unit) const;
    void parameter(uint32_t pname, int32_t value);
    void parameter(uint32_t pname, GLfloat value);
};
} // namespace glxx

inline glxx::Sampler::Sampler(glxx::Sampler &&rhs)
{
    handle = rhs.handle;
    rhs.handle = 0;
}

inline glxx::Sampler &glxx::Sampler::operator=(glxx::Sampler &&rhs)
{
    glxx::Sampler copy {std::move(rhs)};
    std::swap(handle, copy.handle);
    return *this;
}

inline void glxx::Sampler::create()
{
    destroy();
    glCreateSamplers(1, &handle);
}

inline void glxx::Sampler::destroy()
{
    if(handle) {
        glDeleteSamplers(1, &handle);
        handle = 0;
    }
}

inline void glxx::Sampler::bind(uint32_t unit) const
{
    glBindSampler(unit, handle);
}

inline void glxx::Sampler::parameter(uint32_t pname, int32_t value)
{
    glSamplerParameteri(handle, pname, value);
}

inline void glxx::Sampler::parameter(uint32_t pname, GLfloat value)
{
    glSamplerParameterf(handle, pname, value);
}

#endif /* CLIENT_GLXX_SAMPLER_HH */
