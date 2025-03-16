#include "shared/pch.hh"
#include "shared/feature.hh"

#include "shared/chunk.hh"
#include "shared/coord.hh"
#include "shared/dimension.hh"
#include "shared/voxel_storage.hh"

void Feature::place(const voxel_pos &vpos, Dimension *dimension, bool overwrite) const
{
    for(const auto &it : (*this)) {
        auto it_vpos = vpos + it.first;
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

            chunk->set_voxel(it.second, it_index);
        }
    }
}

void Feature::place(const voxel_pos &vpos, const chunk_pos &cpos, VoxelStorage &voxels, bool overwrite) const
{
    for(const auto &it : (*this)) {
        auto it_vpos = vpos + it.first;
        auto it_cpos = coord::to_chunk(it_vpos);

        if(it_cpos == cpos) {
            auto it_lpos = coord::to_local(it_vpos);
            auto it_index = coord::to_index(it_lpos);

            if(voxels[it_index] && !overwrite) {
                // There is something in the way
                // and the called intentionally requested
                // we do not force feature to overwrite voxels
                continue;
            }

            voxels[it_index] = it.second;
        }
    }
}
