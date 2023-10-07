// SPDX-License-Identifier: MPL-2.0
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#ifndef CLIENT_GL_SHADER_HH
#define CLIENT_GL_SHADER_HH
#include <client/gl/object.hh>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace gl
{
class Shader final : public gl::Object<Shader> {
public:
    Shader() = default;
    Shader(Shader &&rhs);
    Shader &operator=(Shader &&rhs);
    void create(uint32_t stage);
    void destroy();
    bool glsl(const std::string &source);
#if 0
    bool spirv(const std::vector<uint8_t> &binary);
#endif
    constexpr uint32_t get_stage() const;

private:
    uint32_t stage {0};
};
} // namespace gl

namespace detail
{
static inline void check_shader_info_log(uint32_t shader)
{
    int32_t length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if(length > 1) {
        std::string info_log(static_cast<size_t>(length) + 1, static_cast<char>(0));
        glGetShaderInfoLog(shader, static_cast<int32_t>(info_log.size()), nullptr, info_log.data());
        spdlog::debug(info_log);
    }
}
} // namespace detail

inline gl::Shader::Shader(gl::Shader &&rhs)
    : stage{rhs.stage}
{
    handle = rhs.handle;
    rhs.handle = 0;
    rhs.stage = 0;
}

inline gl::Shader &gl::Shader::operator=(gl::Shader &&rhs)
{
    gl::Shader copy {std::move(rhs)};
    std::swap(handle, copy.handle);
    std::swap(stage, copy.stage);
    return *this;
}

inline void gl::Shader::create(uint32_t stage)
{
    destroy();
    this->handle = glCreateShader(stage);
    this->stage = stage;
}

inline void gl::Shader::destroy()
{
    if(handle) {
        glDeleteShader(handle);
        handle = 0;
        stage = 0;
    }
}

inline bool gl::Shader::glsl(const std::string &source)
{
    if(handle) {
        int32_t status;
        const char *data = source.c_str();

        glShaderSource(handle, 1, &data, nullptr);
        glCompileShader(handle);
        detail::check_shader_info_log(handle);

        glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
        return status == GL_TRUE;
    }

    return false;
}

#if 0
inline bool gl::Shader::spirv(uint32_t stage, const std::vector<uint8_t> &binary)
{
    if(handle && stage) {
        int32_t status;
        const char *data = source.c_str();

        glShaderBinary(1, &handle, GL_SHADER_BINARY_FORMAT_SPIR_V, binary.data(), static_cast<int32_t>(binary.size()));
        glSpecializeShader(handle, "main", 0, nullptr, nullptr);
        detail::check_shader_info_log(handle);

        glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
        return status;
    }

    return false;
}
#endif

inline constexpr uint32_t gl::Shader::get_stage() const
{
    return stage;
}

#endif/* CLIENT_GL_SHADER_HH */