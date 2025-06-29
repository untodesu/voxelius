#ifndef SHARED_CHUNK_AABB
#define SHARED_CHUNK_AABB 1
#pragma once

#include "core/macros.hh"

#include "shared/types.hh"

class ChunkAABB final {
public:
    DECLARE_DEFAULT_CONSTRUCTOR(ChunkAABB);
    virtual ~ChunkAABB(void) = default;

    void set_bounds(const chunk_pos& min, const chunk_pos& max);
    void set_offset(const chunk_pos& base, const chunk_pos& size);

    const chunk_pos& get_min(void) const;
    const chunk_pos& get_max(void) const;

    bool contains(const chunk_pos& point) const;
    bool intersect(const ChunkAABB& other_box) const;

    ChunkAABB combine_with(const ChunkAABB& other_box) const;
    ChunkAABB multiply_with(const ChunkAABB& other_box) const;
    ChunkAABB push(const chunk_pos& vector) const;

public:
    chunk_pos min;
    chunk_pos max;
};

#endif // SHARED_CHUNK_AABB
