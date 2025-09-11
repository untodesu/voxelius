#include "shared/pch.hh"

#include "shared/world/feature.hh"

#include "shared/world/chunk.hh"
#include "shared/world/dimension.hh"

#include "shared/coord.hh"

void world::Feature::place(const voxel_pos& vpos, Dimension* dimension) const
{
    for(const auto [rpos, voxel, overwrite] : (*this)) {
        auto it_vpos = vpos + rpos;
        auto it_cpos = coord::to_chunk(it_vpos);

        if(auto chunk = dimension->create_chunk(it_cpos)) {
            auto it_lpos = coord::to_local(it_vpos);
            auto it_index = coord::to_index(it_lpos);

            if(chunk->get_voxel(it_index) && !overwrite) {
                // There is something in the way
                // and the called intentionally requested
                // we do not force feature to overwrite voxels
                continue;
            }

            chunk->set_voxel(voxel, it_index);
        }
    }
}

void world::Feature::place(const voxel_pos& vpos, const chunk_pos& cpos, Chunk& chunk) const
{
    for(const auto [rpos, voxel, overwrite] : (*this)) {
        auto it_vpos = vpos + rpos;
        auto it_cpos = coord::to_chunk(it_vpos);

        if(it_cpos == cpos) {
            auto it_lpos = coord::to_local(it_vpos);
            auto it_index = coord::to_index(it_lpos);

            if(chunk.get_voxel(it_index) && !overwrite) {
                // There is something in the way
                // and the called intentionally requested
                // we do not force feature to overwrite voxels
                continue;
            }

            chunk.set_voxel(voxel, it_index);
        }
    }
}
