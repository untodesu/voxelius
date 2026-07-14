#include "shared/pch.hh"

#include "shared/physics.hh"

#include "shared/block_collisions.hh"
#include "shared/block_registry.hh"
#include "shared/ray_aabb.hh"
#include "shared/ray_dda.hh"
#include "shared/utils/coord.hh"
#include "shared/world.hh"

std::optional<physics::Hit> physics::raycast_block(const Ray& ray, block_filter bfilter) noexcept
{
    assert(bfilter);

    std::optional<Hit> closest = std::nullopt;
    auto closest_distance = std::numeric_limits<float>::max();

    Ray_DDA dda(ray.start_chunk, ray.start, ray.direction);

    while(dda.distance() < ray.max_distance) {
        auto block_id = dda.step();

        if(block_id == BLOCK_ID_NULL) {
            continue;
        }

        auto def = block_registry::find_definition(block_id);
        auto bcoll = block_collisions::find(block_id);

        if(def == nullptr || bcoll == nullptr) {
            continue;
        }

        if(bfilter == BLOCK_FILTER_SOLID && (bcoll->elements.empty() || def->touch == BLOCK_TOUCH_NONE)) {
            continue;
        }

        auto hit_cpos = utils::to_chunk(dda.position());
        auto hit_lpos = utils::to_local(dda.position());
        auto bpos_float = utils::to_relative(ray.start_chunk, hit_cpos, hit_lpos.cast<float>());
        auto hit_in_block = false;

        for(const auto& aabb : bcoll->elements) {
            Eigen::AlignedBox3f world_aabb = aabb;
            world_aabb.translate(bpos_float);

            Ray_AABB ray_aabb(ray.start, ray.direction);

            if(auto hit = ray_aabb.intersect(world_aabb)) {
                if(hit->distance <= ray.max_distance) {
                    if(hit->distance < closest_distance) {
                        Hit new_hit {};
                        new_hit.distance = hit->distance;
                        new_hit.normal = hit->normal;
                        new_hit.target = block_id;
                        new_hit.block_pos = dda.position();

                        new_hit.chunk_pos = hit_cpos;
                        new_hit.local_pos = hit_lpos;
                        new_hit.point = utils::to_relative(hit_cpos, ray.start_chunk, hit->point);

                        closest = std::move(new_hit);
                        closest_distance = hit->distance;

                        hit_in_block = true;
                    }
                }
            }
        }

        if(hit_in_block) {
            break;
        }
    }

    return closest;
}

std::optional<physics::Hit> physics::raycast_entity(const Ray& ray, entity_filter efilter) noexcept
{
    assert(efilter);

    return std::nullopt; // TODO
}

std::optional<physics::Hit> physics::raycast(const Ray& ray, block_filter bfilter, entity_filter efilter) noexcept
{
    std::optional<Hit> closest = std::nullopt;

    if(bfilter) {
        closest = raycast_block(ray, bfilter);
    }

    if(efilter) {
        auto hit = raycast_entity(ray, efilter);
        auto hit_distance = std::numeric_limits<float>::max();
        auto closest_distance = std::numeric_limits<float>::max();

        if(hit.has_value()) {
            hit_distance = hit->distance;
        }

        if(closest.has_value()) {
            closest_distance = closest->distance;
        }

        if(hit_distance < closest_distance) {
            closest = std::move(hit);
        }
    }

    return closest;
}
