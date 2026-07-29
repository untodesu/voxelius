#include "client/pch.hh"

#include "client/world/chunk_vbo.hh"

#include "client/world/chunk_mesh.hh"

#include <algorithm>

static constexpr std::size_t INITIAL_CAPACITY = 1024U * 1024U;

GLuint chunk_vbo::handle;
std::size_t chunk_vbo::capacity;
std::vector<chunk_vbo::FreeRegion> chunk_vbo::free_list;

static std::size_t s_vertex_limit;

static void grow(std::size_t required)
{
    static std::vector<std::uint8_t> scratch;

    std::size_t new_capacity = chunk_vbo::capacity;

    while(new_capacity < required) {
        new_capacity <<= 1;
    }

    scratch.resize(new_capacity * sizeof(ChunkMesh_Vertex));

    glBindBuffer(GL_ARRAY_BUFFER, chunk_vbo::handle);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(chunk_vbo::capacity * sizeof(ChunkMesh_Vertex)), scratch.data());
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(new_capacity * sizeof(ChunkMesh_Vertex)), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(chunk_vbo::capacity * sizeof(ChunkMesh_Vertex)), scratch.data());

    chunk_vbo::capacity = new_capacity;
}

static void coalesce(void)
{
    if(1 == chunk_vbo::free_list.size()) {
        return; // nothing to coalesce
    }

    std::sort(chunk_vbo::free_list.begin(), chunk_vbo::free_list.end(), [](const auto& a, const auto& b) {
        return a.base < b.base;
    });

    std::vector<chunk_vbo::FreeRegion> merged;
    merged.reserve(chunk_vbo::free_list.size());

    auto current = chunk_vbo::free_list.front();

    for(std::size_t i = 1; i < chunk_vbo::free_list.size(); ++i) {
        const auto& next = chunk_vbo::free_list[i];

        if(current.base + current.size == next.base) {
            current.size += next.size;
        }
        else {
            merged.push_back(current);
            current = next;
        }
    }

    merged.push_back(current);

    chunk_vbo::free_list = std::move(merged);
}

void chunk_vbo::init(void)
{
    capacity = INITIAL_CAPACITY;
    s_vertex_limit = 0;
    free_list.clear();

    glGenBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(capacity * sizeof(ChunkMesh_Vertex)), nullptr, GL_DYNAMIC_DRAW);
}

void chunk_vbo::shutdown(void)
{
    glDeleteBuffers(1, &handle);

    handle = 0;
    capacity = 0;
    s_vertex_limit = 0;
    free_list.clear();
}

std::uint32_t chunk_vbo::allocate(std::size_t vertex_count)
{
    if(vertex_count == 0) {
        return 0;
    }

    auto it = std::find_if(free_list.begin(), free_list.end(), [vertex_count](const auto& region) {
        return region.size >= vertex_count;
    });

    if(it == free_list.end()) {
        if(s_vertex_limit + vertex_count > capacity) {
            grow(s_vertex_limit + vertex_count);
        }

        auto base = static_cast<std::uint32_t>(s_vertex_limit);
        s_vertex_limit += vertex_count;
        return base;
    }
    else {
        auto base = it->base;

        if(it->size == vertex_count) {
            free_list.erase(it);
        }
        else {
            it->base += static_cast<std::uint32_t>(vertex_count);
            it->size -= static_cast<std::uint32_t>(vertex_count);
        }

        return base;
    }
}

void chunk_vbo::free(std::uint32_t base, std::size_t vertex_count)
{
    if(vertex_count) {
        FreeRegion region {};
        region.base = base;
        region.size = static_cast<std::uint32_t>(vertex_count);

        free_list.emplace_back(std::move(region));

        coalesce();
    }
}

void chunk_vbo::upload(std::uint32_t base, std::span<const ChunkMesh_Vertex> data)
{
    if(data.size()) {
        auto offset = static_cast<GLintptr>(base * sizeof(ChunkMesh_Vertex));
        auto length = static_cast<GLsizeiptr>(data.size_bytes());

        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferSubData(GL_ARRAY_BUFFER, offset, length, data.data());
    }
}
