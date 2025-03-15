#ifndef CLIENT_PROGRAM_HH
#define CLIENT_PROGRAM_HH 1
#pragma once

struct GL_VariedMacro final {
    std::string name;
    unsigned long line;
    unsigned int value;
};

struct GL_Uniform final {
    std::string name;
    GLint location;
};

class GL_Program final {
public:
    bool setup(const char *vpath, const char *fpath);
    void destroy(void);
    bool update(void);

    std::size_t add_uniform(const char *name);
    void set_variant_vert(unsigned int variant, unsigned int value);
    void set_variant_frag(unsigned int variant, unsigned int value);

public:
    std::string vert_path;
    std::string frag_path;
    std::vector<std::string> vert_source;
    std::vector<std::string> frag_source;
    std::vector<GL_VariedMacro> vert_variants;
    std::vector<GL_VariedMacro> frag_variants;
    std::vector<GL_Uniform> uniforms;
    bool needs_update;
    GLuint handle;
};

#endif /* CLIENT_PROGRAM_HH  */
