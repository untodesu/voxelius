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
#include <parson.h>
#include <shared/vdef.hh>
#include <spdlog/spdlog.h>

std::unordered_set<std::string> vdef::names = {};
std::unordered_set<vfs::path_t, vfs::path_hasher_t> vdef::textures = {};
std::vector<VoxelInfoRoot *> vdef::voxels = {};

bool vdef::assign(const std::string &name, uint32_t id)
{
    if(id == NULL_VOXEL_ID) {
        spdlog::warn("vdef: can't assign to NULL_VOXEL_ID");
        return false;
    }

    if(vdef::names.count(name)) {
        spdlog::warn("vdef: name {} is already assigned", name);
        return false;
    }

    std::string source = {};

    const vfs::path_t path = fmt::format("/voxels/{}.json", name);

    if(!vfs::read_string(path, source)) {
        spdlog::warn("vdef: {}: {}", path.string(), vfs::get_error());
        return false;
    }

    JSON_Value *jsonv = json_parse_string(source.c_str());
    JSON_Object *json = json_value_get_object(jsonv);

    if(!jsonv) {
        spdlog::warn("vdef: {}: parse error", path.string());
        return false;
    }

    if(!json) {
        json_value_free(jsonv);
        spdlog::warn("vdef: {}: root is not an object", path.string());
        return false;
    }

    VoxelInfoRoot iroot = {};
    iroot.name = name;
    iroot.vec.clear();

    JSON_Array *states = json_object_get_array(json, "states");
    size_t num_states = json_array_get_count(states);

    if(num_states == 0) {
        json_value_free(jsonv);
        spdlog::warn("vdef: {}: voxel has no states", path.string());
        return false;
    }

    for(size_t i = 0; i < num_states; ++i) {
        VoxelInfo info = {};
        info.draw = VOXEL_DRAW_NODRAW;
        info.name = iroot.name;

        for(size_t j = 0; j < NUM_VOXEL_FACE; ++j) {
            info.textures[j].paths.clear();
            info.textures[j].offset = UINT16_MAX;
        }

        JSON_Object *pstate = json_array_get_object(states, i);

        if(!pstate) {
            json_value_free(jsonv);
            spdlog::warn("vdef: {}: state #{} is not an object", path.string(), i);
            return false;
        }

        if(const char *str = json_object_get_string(pstate, "draw")) {
            static const std::unordered_map<std::string, voxel_draw_t> draws = {
                { "nodraw", VOXEL_DRAW_NODRAW   },
                { "solid",  VOXEL_DRAW_SOLID    },
                { "cutout", VOXEL_DRAW_CUTOUT   },
                { "blend",  VOXEL_DRAW_BLEND    },
            };

            if(const auto it = draws.find(str); it != draws.cend()) {
                info.draw = it->second;
            }
        }

        JSON_Object *textures = json_object_get_object(pstate, "textures");
        size_t num_textures = json_object_get_count(textures);

        for(size_t j = 0; j < num_textures; ++j) {
            const char *fstr = json_object_get_name(textures, j);
            static const std::unordered_map<std::string, voxel_face_t> faces = {
                { "north",  VOXEL_FACE_NORTH    },
                { "south",  VOXEL_FACE_SOUTH    },
                { "east",   VOXEL_FACE_EAST     },
                { "west",   VOXEL_FACE_WEST     },
                { "top",    VOXEL_FACE_TOP      },
                { "bottom", VOXEL_FACE_BOTTOM   },
            };

            const auto it = faces.find(fstr);
            if(it == faces.cend())
                continue;
            const auto face = it->second;

            JSON_Value *pathsv = json_object_get_value_at(textures, j);
            JSON_Array *paths = json_value_get_array(pathsv);
            size_t num_paths = json_array_get_count(paths);

            for(size_t k = 0; k < num_paths; ++k) {
                if(const char *tstr = json_array_get_string(paths, k)) {
                    if(tstr[0] == '^') {
                        if(const char *qstr = json_object_get_string(json, &tstr[1])) {
                            info.textures[face].paths.push_back(qstr);
                            vdef::textures.emplace(qstr);
                            continue;
                        }
                    }

                    info.textures[face].paths.push_back(tstr);
                    vdef::textures.emplace(tstr);
                }
            }
        }

        iroot.vec.push_back(info);
    }

    if(vdef::voxels.size() <= id)
        vdef::voxels.resize(id + 1, nullptr);
    vdef::voxels[id] = new VoxelInfoRoot{iroot};

    vdef::names.emplace(name);

    return true;
}

VoxelInfo *vdef::find(uint32_t id, uint8_t state)
{
    if(id >= vdef::voxels.size() || !vdef::voxels[id])
        return nullptr;
    if(state >= vdef::voxels[id]->vec.size())
        return nullptr;
    return &vdef::voxels[id]->vec[state];
}

VoxelInfo *vdef::find(voxel_t voxel)
{
    const auto id = get_voxel_id(voxel);
    const auto state = get_voxel_state(voxel);
    return vdef::find(id, state);
}

void vdef::purge()
{
    for(auto it : vdef::voxels)
        delete it;
    vdef::names.clear();
    vdef::textures.clear();
    vdef::voxels.clear();
}
