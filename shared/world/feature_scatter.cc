#include "shared/pch.hh"

#include "shared/world/feature_scatter.hh"

#include "shared/constant.hh"
#include "shared/res/feature.hh"
#include "shared/utils/coord.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/climate.hh"
#include "shared/world/climate_noise.hh"
#include "shared/world/entropy_cache.hh"
#include "shared/world/heightmap.hh"

static float scatter_roll(std::uint64_t entropy, std::size_t entry_index, const Identifier& feature_id)
{
    auto roll_seed = entropy;
    roll_seed ^= static_cast<std::uint64_t>(entry_index) * UINT64_C(0x9E3779B97F4A7C15);
    roll_seed ^= std::hash<Identifier>()(feature_id);
    roll_seed ^= roll_seed >> 33;
    roll_seed *= UINT64_C(0xFF51AFD7ED558CCD);
    roll_seed ^= roll_seed >> 33;
    return static_cast<float>(roll_seed >> 33) / static_cast<float>(1ULL << 31);
}

static block_id_type block_at(const PlacementContext& context, const BlockPos& bpos, block_id_type fallback)
{
    if(context.storage && context.position == utils::to_chunk(bpos)) {
        return context.storage->get(utils::to_local(bpos));
    }

    return fallback;
}

static bool matches_tags(block_id_type block_id, block_tag_bit required)
{
    if(required == static_cast<block_tag_bit>(0)) {
        return true;
    }

    if(block_id == BLOCK_ID_NULL) {
        return static_cast<bool>(required & BLOCK_TAG_GAS);
    }

    return block_registry::has_tag_any(block_id, required);
}

static bool can_place(const PlacementContext& context, const BlockPos& origin, const BiomeDefinition& biome, const BiomeScatterEntry& entry,
    const Column& column)
{
    if(!column.is_valid()) {
        return false;
    }

    if(origin.y() != column.surface_y) {
        return false;
    }

    if(column.liquid_y >= column.surface_y) {
        return false;
    }

    auto above = BlockPos(origin.x(), origin.y() + 1, origin.z());
    auto in_chunk = utils::to_chunk(origin) == context.position;

    auto origin_block = block_at(context, origin, biome.palette_surface.cached);
    auto above_block = block_at(context, above, BLOCK_ID_NULL);

    if(origin_block == biome.palette_fluid.cached) {
        return false;
    }

    if(in_chunk && above_block == biome.palette_fluid.cached) {
        return false;
    }

    if(entry.need_below != static_cast<block_tag_bit>(0) && !matches_tags(origin_block, entry.need_below)) {
        return false;
    }

    if(entry.need_above != static_cast<block_tag_bit>(0)) {
        if(in_chunk && !matches_tags(above_block, entry.need_above)) {
            return false;
        }
    }

    return true;
}

static bool check_footprint(biome_realm realm, BlockPos::value_type bx, BlockPos::value_type bz, const Feature& feature, unsigned edge,
    const BiomeDefinition* biome)
{
    auto& bounds = feature.bounds;
    auto pad = static_cast<int>(edge);

    auto min_x = bounds.min().x() - pad;
    auto max_x = bounds.max().x() + pad;
    auto min_z = bounds.min().z() - pad;
    auto max_z = bounds.max().z() + pad;

    for(auto dz = min_z; dz <= max_z; dz += 1) {
        for(auto dx = min_x; dx <= max_x; dx += 1) {
            BlockPosXZ sample_pos;
            sample_pos[0] = bx + dx;
            sample_pos[1] = bz + dz;

            if(biome == climate::find(realm, climate_noise::sample_block(sample_pos)))
                continue;
            return false;
        }
    }

    return true;
}

static bool would_place(BlockPos::value_type bx, BlockPos::value_type bz, const PlacementContext& context, const BiomeDefinition& biome,
    const BiomeScatterEntry& entry, std::size_t entry_index)
{
    if(entry.cached == nullptr) {
        return false;
    }

    ChunkPosXZ owner_xz;
    owner_xz[0] = static_cast<ChunkPosXZ::value_type>(bx >> constant::CHUNK_SIZE_LOG2);
    owner_xz[1] = static_cast<ChunkPosXZ::value_type>(bz >> constant::CHUNK_SIZE_LOG2);

    auto& entropy_array = entropy_cache::get(owner_xz);
    auto entropy_lx = static_cast<std::size_t>(bx & (constant::CHUNK_SIZE - 1));
    auto entropy_lz = static_cast<std::size_t>(bz & (constant::CHUNK_SIZE - 1));
    auto entropy_index = entropy_lx + entropy_lz * constant::CHUNK_SIZE;
    auto entropy = entropy_array[entropy_index];

    if(scatter_roll(entropy, entry_index, entry.feature) >= entry.chance) {
        return false;
    }

    BlockPosXZ sample_xz;
    sample_xz[0] = bx;
    sample_xz[1] = bz;

    auto& column = heightmap::probe_slow(context.realm, sample_xz);
    auto position = BlockPos(bx, column.surface_y, bz);

    if(!can_place(context, position, biome, entry, column)) {
        return false;
    }

    return check_footprint(context.realm, bx, bz, *entry.cached, entry.edge, &biome);
}

static BlockAlignedBox scatter_bounds(const BlockPos& origin, const Feature& feature, block_face facing)
{
    const auto bounds = feature_placer::rotate_bounds(feature.bounds, facing);

    BlockAlignedBox world;
    world.min() = origin + bounds.min().cast<BlockPos::value_type>();
    world.max() = origin + bounds.max().cast<BlockPos::value_type>();
    return world;
}

static BlockPos::value_type scatter_extent(const Feature& feature)
{
    const auto& bounds = feature.bounds;
    const auto min_xz = Eigen::Vector2i(bounds.min().x(), bounds.min().z());
    const auto max_xz = Eigen::Vector2i(bounds.max().x(), bounds.max().z());
    return static_cast<BlockPos::value_type>(std::max(min_xz.cwiseAbs().maxCoeff(), max_xz.cwiseAbs().maxCoeff()));
}

static bool bounds_too_close(const BlockAlignedBox& a, const BlockAlignedBox& b, unsigned padding)
{
    if(padding == 0U) {
        return false;
    }

    BlockAlignedBox expanded = a;
    expanded.min() -= BlockPos::Constant(padding);
    expanded.max() += BlockPos::Constant(padding);
    return expanded.intersects(b);
}

static bool same_group(const BiomeScatterEntry& a, const BiomeScatterEntry& b, std::size_t a_index, std::size_t b_index)
{
    if(a.group_hash) {
        return a.group_hash == b.group_hash;
    }

    return a_index == b_index;
}

static const BiomeScatterEntry* find_group_winner(BlockPos::value_type bx, BlockPos::value_type bz, const PlacementContext& context,
    const BiomeDefinition& biome, const BiomeScatterEntry& entry, std::size_t entry_index)
{
    const BiomeScatterEntry* winner = nullptr;

    for(std::size_t i = 0; i < biome.scatter.size(); i += 1) {
        const auto& candidate = biome.scatter[i];

        if(!same_group(entry, candidate, entry_index, i)) {
            continue;
        }

        if(!would_place(bx, bz, context, biome, candidate, i)) {
            continue;
        }

        winner = &candidate;
    }

    return winner;
}

static BlockPos::value_type group_search_radius(const BiomeDefinition& biome, const BiomeScatterEntry& entry, std::size_t entry_index)
{
    auto search = static_cast<BlockPos::value_type>(entry.padding) + scatter_extent(*entry.cached);

    for(std::size_t i = 0; i < biome.scatter.size(); i += 1) {
        auto& candidate = biome.scatter[i];

        if(candidate.cached == nullptr) {
            continue;
        }

        if(!same_group(entry, candidate, entry_index, i)) {
            continue;
        }

        auto candidate_search = static_cast<BlockPos::value_type>(candidate.padding);
        candidate_search += scatter_extent(*candidate.cached);
        search = std::max(search, candidate_search);
    }

    return search;
}

static bool resolve_padding_tie(BlockPos::value_type bx, BlockPos::value_type bz, const Column& column, const PlacementContext& context,
    const BiomeDefinition& biome, const BiomeScatterEntry& entry, std::size_t entry_index)
{
    if(entry.padding == 0U || entry.cached == nullptr) {
        return false;
    }

    auto& feature = *entry.cached;
    auto origin = BlockPos(bx, column.surface_y, bz);
    auto current_box = scatter_bounds(origin, feature, BLOCK_FACE_NORTH);
    auto search = group_search_radius(biome, entry, entry_index);

    for(auto dx = -search; dx <= search; dx += 1) {
        for(auto dz = -search; dz <= search; dz += 1) {
            if(dx == 0 && dz == 0) {
                continue;
            }

            auto ox = bx + dx;
            auto oz = bz + dz;

            auto neighbour_entry = find_group_winner(ox, oz, context, biome, entry, entry_index);

            if(neighbour_entry == nullptr || neighbour_entry->cached == nullptr) {
                continue;
            }

            BlockPosXZ neighbour_xz;
            neighbour_xz[0] = ox;
            neighbour_xz[1] = oz;

            auto& neighbour_column = heightmap::probe_slow(context.realm, neighbour_xz);

            if(!neighbour_column.is_valid()) {
                continue;
            }

            auto neighbour_origin = BlockPos(ox, neighbour_column.surface_y, oz);
            auto neighbour_bounds = scatter_bounds(neighbour_origin, *neighbour_entry->cached, BLOCK_FACE_NORTH);

            if(!bounds_too_close(current_box, neighbour_bounds, entry.padding)) {
                continue;
            }

            if(ox < bx || (ox == bx && oz < bz)) {
                return true;
            }
        }
    }

    return false;
}

void FeatureScatter::init(void)
{
    feature_placer::add_rule<FeatureScatter>();
}

void FeatureScatter::collect(const ChunkPos& pos, const PlacementContext& context, std::vector<FeatureInstance>& out) const
{
    auto extent = static_cast<BlockPos::value_type>(context.max_extent);
    auto chunk_origin = utils::to_block(pos);
    auto chunk_max_x = chunk_origin.x() + static_cast<BlockPos::value_type>(constant::CHUNK_SIZE - 1);
    auto chunk_max_z = chunk_origin.z() + static_cast<BlockPos::value_type>(constant::CHUNK_SIZE - 1);

    auto search_min_x = chunk_origin.x() - extent;
    auto search_max_x = chunk_max_x + extent;
    auto search_min_z = chunk_origin.z() - extent;
    auto search_max_z = chunk_max_z + extent;

    for(auto wx = search_min_x; wx <= search_max_x; wx += 1) {
        for(auto wz = search_min_z; wz <= search_max_z; wz += 1) {
            BlockPosXZ sample_pos;
            sample_pos.x() = wx;
            sample_pos.y() = wz;

            auto sample = climate_noise::sample_block(sample_pos);
            auto biome = climate::find(context.realm, sample);

            if(biome == nullptr || biome->scatter.empty()) {
                continue;
            }

            auto& column = heightmap::probe_slow(context.realm, sample_pos);

            if(!column.is_valid()) {
                continue;
            }

            BlockPos origin;
            origin.x() = wx;
            origin.y() = column.surface_y;
            origin.z() = wz;

            std::optional<FeatureInstance> column_winner;

            for(std::size_t i = 0; i < biome->scatter.size(); ++i) {
                auto& entry = biome->scatter[i];

                if(!would_place(wx, wz, context, *biome, entry, i)) {
                    continue;
                }

                if(resolve_padding_tie(wx, wz, column, context, *biome, entry, i)) {
                    continue;
                }

                FeatureInstance instance {};
                instance.origin = origin;
                instance.feature = entry.cached;
                instance.facing = BLOCK_FACE_NORTH;
                column_winner = std::move(instance);
            }

            if(column_winner.has_value()) {
                out.push_back(std::move(column_winner.value()));
            }
        }
    }
}
