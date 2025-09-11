#include "shared/pch.hh"

#include "shared/entity/collision.hh"

#include "core/math/constexpr.hh"

#include "shared/entity/gravity.hh"
#include "shared/entity/grounded.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"
#include "shared/world/voxel_registry.hh"

#include "shared/coord.hh"
#include "shared/globals.hh"

static int vgrid_collide(const world::Dimension* dimension, int d, entity::Collision& collision, entity::Transform& transform,
    entity::Velocity& velocity, world::voxel_surface& touch_surface)
{
    const auto move = globals::fixed_frametime * velocity.value[d];
    const auto move_sign = math::sign<int>(move);

    const auto& ref_aabb = collision.aabb;
    const auto current_aabb = ref_aabb.push(transform.local);

    auto next_aabb = math::AABBf(current_aabb);
    next_aabb.min[d] += move;
    next_aabb.max[d] += move;

    local_pos lpos_min;
    lpos_min.x = math::floor<local_pos::value_type>(next_aabb.min.x);
    lpos_min.y = math::floor<local_pos::value_type>(next_aabb.min.y);
    lpos_min.z = math::floor<local_pos::value_type>(next_aabb.min.z);

    local_pos lpos_max;
    lpos_max.x = math::ceil<local_pos::value_type>(next_aabb.max.x);
    lpos_max.y = math::ceil<local_pos::value_type>(next_aabb.max.y);
    lpos_max.z = math::ceil<local_pos::value_type>(next_aabb.max.z);

    // Other axes
    const int u = (d + 1) % 3;
    const int v = (d + 2) % 3;

    local_pos::value_type ddir;
    local_pos::value_type dmin;
    local_pos::value_type dmax;

    if(move < 0.0f) {
        ddir = local_pos::value_type(+1);
        dmin = lpos_min[d];
        dmax = lpos_max[d];
    }
    else {
        ddir = local_pos::value_type(-1);
        dmin = lpos_max[d];
        dmax = lpos_min[d];
    }

    world::voxel_touch latch_touch = world::voxel_touch::NOTHING;
    glm::fvec3 latch_values = glm::fvec3(0.0f, 0.0f, 0.0f);
    world::voxel_surface latch_surface = world::voxel_surface::UNKNOWN;
    math::AABBf latch_vbox;

    for(auto i = dmin; i != dmax; i += ddir) {
        for(auto j = lpos_min[u]; j < lpos_max[u]; ++j)
            for(auto k = lpos_min[v]; k < lpos_max[v]; ++k) {
                local_pos lpos;
                lpos[d] = i;
                lpos[u] = j;
                lpos[v] = k;

                const auto vpos = coord::to_voxel(transform.chunk, lpos);
                const auto info = world::voxel_registry::find(dimension->get_voxel(vpos));

                if(info == nullptr) {
                    // Don't collide with something
                    // that we assume to be nothing
                    continue;
                }

                math::AABBf vbox;
                vbox.min = glm::fvec3(lpos);
                vbox.max = glm::fvec3(lpos) + 1.0f;

                if(!next_aabb.intersect(vbox)) {
                    // No intersection between the voxel
                    // and the entity's collision hull
                    continue;
                }

                if(info->touch_type == world::voxel_touch::SOLID) {
                    // Solid touch type makes a collision
                    // response whenever it is encountered
                    velocity.value[d] = 0.0f;
                    touch_surface = info->surface;
                    return move_sign;
                }

                // In case of other touch types, they
                // are latched and the last ever touch
                // type is then responded to
                if(info->touch_type != world::voxel_touch::NOTHING) {
                    latch_touch = info->touch_type;
                    latch_values = info->touch_values;
                    latch_surface = info->surface;
                    latch_vbox = vbox;
                    continue;
                }
            }
    }

    if(latch_touch != world::voxel_touch::NOTHING) {
        if(latch_touch == world::voxel_touch::BOUNCE) {
            const auto move_distance = math::abs(current_aabb.min[d] - next_aabb.min[d]);
            const auto threshold = 2.0f * globals::fixed_frametime;

            if(move_distance > threshold) {
                velocity.value[d] *= -latch_values[d];
            }
            else {
                velocity.value[d] = 0.0f;
            }

            touch_surface = latch_surface;

            return move_sign;
        }

        if(latch_touch == world::voxel_touch::SINK) {
            velocity.value[d] *= latch_values[d];
            touch_surface = latch_surface;
            return move_sign;
        }
    }

    return 0;
}

void entity::Collision::fixed_update(world::Dimension* dimension)
{
    // FIXME: this isn't particularly accurate considering
    // some voxels might be passable and some other voxels
    // might apply some slowing factor; what I might do in the
    // future is to add a specific value to the voxel registry
    // entries that would specify the amount of force we apply
    // to prevent player movement inside a specific voxel, plus
    // we shouldn't treat all voxels as full cubes if we want
    // to support slabs, stairs and non-full liquid voxels in the future

    auto group = dimension->entities.group<entity::Collision>(entt::get<entity::Transform, entity::Velocity>);

    for(auto [entity, collision, transform, velocity] : group.each()) {
        auto surface = world::voxel_surface::UNKNOWN;
        auto vertical_move = vgrid_collide(dimension, 1, collision, transform, velocity, surface);

        if(dimension->entities.any_of<entity::Gravity>(entity)) {
            if(vertical_move == math::sign<int>(dimension->get_gravity())) {
                dimension->entities.emplace_or_replace<entity::Grounded>(entity, entity::Grounded { surface });
            }
            else {
                dimension->entities.remove<entity::Grounded>(entity);
            }
        }
        else {
            // The entity cannot be grounded because the component
            // setup of said entity should not let it comprehend the
            // concept of resting on the ground (it flies around)
            dimension->entities.remove<entity::Grounded>(entity);
        }

        vgrid_collide(dimension, 0, collision, transform, velocity, surface);
        vgrid_collide(dimension, 2, collision, transform, velocity, surface);
    }
}
