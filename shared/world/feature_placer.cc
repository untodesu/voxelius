#include "shared/pch.hh"

#include "shared/world/feature_placer.hh"

#include "shared/constant.hh"
#include "shared/res/feature.hh"
#include "shared/utils/coord.hh"
#include "shared/world/biome_registry.hh"
#include "shared/world/block_registry.hh"

static std::size_t s_max_extent;
static std::vector<std::unique_ptr<PlacementRule>> s_rules;

static Eigen::Vector3i rotate_vector(const Eigen::Vector3i& vector, block_face facing)
{
    switch(facing) {
        case BLOCK_FACE_SOUTH:
            return Eigen::Vector3i(-vector.x(), vector.y(), -vector.z());

        case BLOCK_FACE_EAST:
            return Eigen::Vector3i(vector.z(), vector.y(), -vector.x());

        case BLOCK_FACE_WEST:
            return Eigen::Vector3i(-vector.z(), vector.y(), vector.x());
    }

    return vector;
}

static bool check_intersection(const FeatureInstance& instance, const ChunkPos& cpos)
{
    if(instance.feature == nullptr) {
        return false;
    }

    auto bounds = feature_placer::rotate_bounds(instance.feature->bounds, instance.facing);
    auto chunk_origin = utils::to_block(cpos);

    Eigen::AlignedBox3i chunk_bounds;
    chunk_bounds.min() = Eigen::Vector3i::Zero();
    chunk_bounds.max() = Eigen::Vector3i::Constant(constant::CHUNK_SIZE - 1);

    BlockAlignedBox block_bounds;
    block_bounds.min() = instance.origin + bounds.min().cast<BlockPos::value_type>();
    block_bounds.max() = instance.origin + bounds.max().cast<BlockPos::value_type>();

    auto local_min = block_bounds.min() - chunk_origin;
    auto local_max = block_bounds.max() - chunk_origin;

    Eigen::AlignedBox3i local_bounds;
    local_bounds.min() = local_min.cast<Eigen::Vector3i::Scalar>();
    local_bounds.max() = local_max.cast<Eigen::Vector3i::Scalar>();

    return chunk_bounds.intersects(local_bounds);
}

static void collect_candidates(const ChunkPos& cpos, biome_realm realm, const BlockStorage& storage, std::vector<FeatureInstance>& out)
{
    PlacementContext context {};
    context.realm = realm;
    context.position = cpos;
    context.storage = &storage;
    context.max_extent = s_max_extent;

    for(auto& rule : s_rules) {
        rule->collect(cpos, context, out);
    }
}

static void commit_instance(BlockStorage& storage, const ChunkPos& cpos, const FeatureInstance& instance)
{
    if(instance.feature == nullptr) {
        return;
    }

    for(auto& part : instance.feature->parts) {
        auto offset = feature_placer::rotate_offset(part.offset, instance.facing);
        auto bpos = instance.origin + offset.cast<BlockPos::value_type>();

        if(cpos == utils::to_chunk(bpos)) {
            auto lpos = utils::to_local(bpos);
            auto existing = storage.get(lpos);

            if(existing == BLOCK_ID_NULL) {
                storage.set(lpos, part.block);
                continue;
            }

            if(block_registry::has_tag_any(existing, part.overwrite)) {
                storage.set(lpos, part.block);
                continue;
            }
        }
    }
}

void feature_placer::detail::add_rule(std::unique_ptr<PlacementRule> rule)
{
    s_rules.push_back(std::move(rule));
}

Eigen::Vector3i feature_placer::rotate_offset(const Eigen::Vector3i& offset, block_face facing)
{
    return rotate_vector(offset, facing);
}

Eigen::AlignedBox3i feature_placer::rotate_bounds(const Eigen::AlignedBox3i& bounds, block_face facing)
{
    if(facing == BLOCK_FACE_NORTH) {
        return bounds;
    }

    Eigen::AlignedBox3i rotated;
    rotated.setEmpty();

    for(Eigen::Index i = 0; i < 8; ++i) {
        auto corner_type = static_cast<Eigen::AlignedBox3i::CornerType>(i);
        auto corner_ident = bounds.corner(corner_type);
        auto corner_rotated = rotate_vector(corner_ident, facing);
        rotated.extend(corner_rotated);
    }

    return rotated;
}

void feature_placer::init(void)
{
    s_max_extent = 0;

    for(auto& def : biome_registry::all_definitions()) {
        for(auto& entry : def.scatter) {
            if(entry.cached == nullptr) {
                continue;
            }

            auto& bounds = entry.cached->bounds;
            Eigen::Vector2i min_xz = Eigen::Vector2i(bounds.min().x(), bounds.min().z());
            Eigen::Vector2i max_xz = Eigen::Vector2i(bounds.max().x(), bounds.max().z());
            auto horizontal = std::max(min_xz.cwiseAbs().maxCoeff(), max_xz.cwiseAbs().maxCoeff());

            s_max_extent = std::max(s_max_extent, entry.padding + entry.edge + static_cast<std::size_t>(horizontal));
        }
    }
}

void feature_placer::shutdown(void)
{
    s_max_extent = 0;
    s_rules.clear();
}

void feature_placer::commit(BlockStorage& storage, const ChunkPos& cpos, biome_realm realm)
{
    thread_local std::vector<FeatureInstance> candidates;

    candidates.clear();
    collect_candidates(cpos, realm, storage, candidates);

    commit(storage, cpos, realm, candidates);
}

void feature_placer::commit(BlockStorage& storage, const ChunkPos& cpos, biome_realm realm, std::span<const FeatureInstance> instances)
{
    for(auto& instance : instances) {
        if(check_intersection(instance, cpos)) {
            commit_instance(storage, cpos, instance);
        }
    }
}
