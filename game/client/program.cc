#include "client/pch.hh"

#include "client/program.hh"

#include "core/utils/string.hh"

// This fills up the array of source lines and figures out
// which lines are to be dynamically resolved as variant macros
static void parse_source(const char* source, std::vector<std::string>& out_lines, std::vector<GL_VariedMacro>& out_variants)
{
    std::string line;
    std::istringstream stream = std::istringstream(source);
    unsigned long line_number = 0UL;

    out_lines.clear();
    out_variants.clear();

    while(std::getline(stream, line)) {
        unsigned int macro_index = {};
        char macro_name[128] = {};

        if(std::sscanf(line.c_str(), " # pragma variant [ %u ] %127[^, \"\t\r\n]", &macro_index, &macro_name) == 2) {
            if(out_variants.size() <= macro_index) {
                out_variants.resize(macro_index + 1U);
            }

            out_variants[macro_index].name = macro_name;
            out_variants[macro_index].line = line_number;
            out_variants[macro_index].value = std::numeric_limits<unsigned int>::max();

            out_lines.push_back(std::string());
            line_number += 1UL;
        } else {
            out_lines.push_back(line);
            line_number += 1UL;
        }
    }
}

static GLuint compile_shader(const char* path, const char* source, GLenum shader_stage)
{
    GLuint shader = glCreateShader(shader_stage);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint info_log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);

    if(info_log_length >= 2) {
        std::basic_string<GLchar> info_log;
        info_log.resize(info_log_length);
        glGetShaderInfoLog(shader, info_log_length, nullptr, info_log.data());
        spdlog::info("gl_program: {}: shader information:", path);
        spdlog::info(info_log);
    }

    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

    if(!compile_status) {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool GL_Program::setup(const char* vpath, const char* fpath)
{
    destroy();

    vert_path = std::string(vpath);
    frag_path = std::string(fpath);

    auto vfile = PHYSFS_openRead(vpath);

    if(vfile == nullptr) {
        spdlog::warn("gl_program: {}: {}", vpath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return false;
    }

    auto vsource = std::string(PHYSFS_fileLength(vfile), char(0x00));
    PHYSFS_readBytes(vfile, vsource.data(), vsource.size());
    PHYSFS_close(vfile);

    auto ffile = PHYSFS_openRead(fpath);

    if(ffile == nullptr) {
        spdlog::warn("gl_program: {}: {}", fpath, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return false;
    }

    auto fsource = std::string(PHYSFS_fileLength(ffile), char(0x00));
    PHYSFS_readBytes(ffile, fsource.data(), fsource.size());
    PHYSFS_close(ffile);

    parse_source(vsource.c_str(), vert_source, vert_variants);
    parse_source(fsource.c_str(), frag_source, frag_variants);

    needs_update = true;
    handle = 0;

    return true;
}

bool GL_Program::update(void)
{
    if(!needs_update) {
        // The program is already up to
        // date with the internal state
        return true;
    }

    for(const auto& macro : vert_variants)
        vert_source[macro.line] = std::format("#define {} {}", macro.name, macro.value);
    for(const auto& macro : frag_variants)
        frag_source[macro.line] = std::format("#define {} {}", macro.name, macro.value);

    std::string vsource(utils::join(vert_source, "\r\n"));
    std::string fsource(utils::join(frag_source, "\r\n"));

    GLuint vert = compile_shader(vert_path.c_str(), vsource.c_str(), GL_VERTEX_SHADER);
    GLuint frag = compile_shader(frag_path.c_str(), fsource.c_str(), GL_FRAGMENT_SHADER);

    if(!vert || !frag) {
        // needs_update = false;
        glDeleteShader(frag);
        glDeleteShader(vert);
        return false;
    }

    handle = glCreateProgram();
    glAttachShader(handle, vert);
    glAttachShader(handle, frag);
    glLinkProgram(handle);

    GLint info_log_length;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &info_log_length);

    if(info_log_length >= 2) {
        std::basic_string<GLchar> info_log;
        info_log.resize(info_log_length);
        glGetProgramInfoLog(handle, info_log_length, nullptr, info_log.data());
        spdlog::info("gl_program: [{}; {}]: program information", vert, frag);
        spdlog::info(info_log);
    }

    glDeleteShader(frag);
    glDeleteShader(vert);

    GLint link_status;
    glGetProgramiv(handle, GL_LINK_STATUS, &link_status);

    if(!link_status) {
        // needs_update = false;
        glDeleteProgram(handle);
        return false;
    }

    for(auto& uniform : uniforms) {
        // NOTE: GL seems to silently ignore invalid uniform
        // locations (-1); should we write something into logs about this?
        uniform.location = glGetUniformLocation(handle, uniform.name.c_str());
    }

    needs_update = false;
    return true;
}

void GL_Program::destroy(void)
{
    if(handle) {
        glDeleteProgram(handle);
        handle = 0;
    }

    uniforms.clear();

    frag_variants.clear();
    frag_source.clear();
    frag_path = std::string();

    vert_variants.clear();
    vert_source.clear();
    vert_path = std::string();

    needs_update = false;
}

std::size_t GL_Program::add_uniform(const char* name)
{
    for(std::size_t i = 0; i < uniforms.size(); ++i) {
        if(!uniforms[i].name.compare(name)) {
            return i;
        }
    }

    const std::size_t index = uniforms.size();
    uniforms.push_back(GL_Uniform());
    uniforms[index].location = -1;
    uniforms[index].name = name;
    return index;
}

void GL_Program::set_variant_vert(unsigned int variant, unsigned int value)
{
    if(variant < vert_variants.size()) {
        if(value != vert_variants[variant].value) {
            vert_variants[variant].value = value;
            needs_update = true;
        }
    }
}

void GL_Program::set_variant_frag(unsigned int variant, unsigned int value)
{
    if(variant < frag_variants.size()) {
        if(value != frag_variants[variant].value) {
            frag_variants[variant].value = value;
            needs_update = true;
        }
    }
}
