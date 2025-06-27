#include "shared/pch.hh"

#include "shared/collision.hh"

#include "core/constexpr.hh"

#include "shared/coord.hh"
#include "shared/dimension.hh"
#include "shared/globals.hh"
#include "shared/gravity.hh"
#include "shared/grounded.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"
#include "shared/voxel_registry.hh"

static int vgrid_collide(const Dimension* dimension, int d, CollisionComponent& collision, TransformComponent& transform,
    VelocityComponent& velocity, voxel_surface& touch_surface)
{
    const auto move = globals::fixed_frametime * velocity.value[d];
    const auto move_sign = vx::sign<int>(move);

    const auto& ref_aabb = collision.aabb;
    const auto current_aabb = ref_aabb.push(transform.local);

    auto next_aabb = AABB(current_aabb);
    next_aabb.min[d] += move;
    next_aabb.max[d] += move;

    local_pos lpos_min;
    lpos_min.x = vx::floor<local_pos::value_type>(next_aabb.min.x);
    lpos_min.y = vx::floor<local_pos::value_type>(next_aabb.min.y);
    lpos_min.z = vx::floor<local_pos::value_type>(next_aabb.min.z);

    local_pos lpos_max;
    lpos_max.x = vx::ceil<local_pos::value_type>(next_aabb.max.x);
    lpos_max.y = vx::ceil<local_pos::value_type>(next_aabb.max.y);
    lpos_max.z = vx::ceil<local_pos::value_type>(next_aabb.max.z);

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
    } else {
        ddir = local_pos::value_type(-1);
        dmin = lpos_max[d];
        dmax = lpos_min[d];
    }

    voxel_touch latch_touch = voxel_touch::NOTHING;
    glm::fvec3 latch_values = glm::fvec3(0.0f, 0.0f, 0.0f);
    voxel_surface latch_surface = voxel_surface::UNKNOWN;
    AABB latch_vbox;

    for(auto i = dmin; i != dmax; i += ddir) {
        for(auto j = lpos_min[u]; j < lpos_max[u]; ++j)
            for(auto k = lpos_min[v]; k < lpos_max[v]; ++k) {
                local_pos lpos;
                lpos[d] = i;
                lpos[u] = j;
                lpos[v] = k;

                const auto vpos = coord::to_voxel(transform.chunk, lpos);
                const auto info = voxel_registry::find(dimension->get_voxel(vpos));

                if(info == nullptr) {
                    // Don't collide with something
                    // that we assume to be nothing
                    continue;
                }

                AABB vbox;
                vbox.min = glm::fvec3(lpos);
                vbox.max = glm::fvec3(lpos) + 1.0f;

                if(!next_aabb.intersect(vbox)) {
                    // No intersection between the voxel
                    // and the entity's collision hull
                    continue;
                }

                if(info->touch_type == voxel_touch::SOLID) {
                    // Solid touch type makes a collision
                    // response whenever it is encountered
                    velocity.value[d] = 0.0f;
                    touch_surface = info->surface;
                    return move_sign;
                }

                // In case of other touch types, they
                // are latched and the last ever touch
                // type is then responded to
                if(info->touch_type != voxel_touch::NOTHING) {
                    latch_touch = info->touch_type;
                    latch_values = info->touch_values;
                    latch_surface = info->surface;
                    latch_vbox = vbox;
                    continue;
                }
            }
    }

    if(latch_touch != voxel_touch::NOTHING) {
        if(latch_touch == voxel_touch::BOUNCE) {
            const auto move_distance = vx::abs(current_aabb.min[d] - next_aabb.min[d]);
            const auto threshold = 2.0f * globals::fixed_frametime;

            if(move_distance > threshold) {
                velocity.value[d] *= -latch_values[d];
            } else {
                velocity.value[d] = 0.0f;
            }

            touch_surface = latch_surface;

            return move_sign;
        }

        if(latch_touch == voxel_touch::SINK) {
            velocity.value[d] *= latch_values[d];
            touch_surface = latch_surface;
            return move_sign;
        }
    }

    return 0;
}

void CollisionComponent::fixed_update(Dimension* dimension)
{
    // FIXME: this isn't particularly accurate considering
    // some voxels might be passable and some other voxels
    // might apply some slowing factor; what I might do in the
    // future is to add a specific value to the voxel registry
    // entries that would specify the amount of force we apply
    // to prevent player movement inside a specific voxel, plus
    // we shouldn't treat all voxels as full cubes if we want
    // to support slabs, stairs and non-full liquid voxels in the future

    auto group = dimension->entities.group<CollisionComponent>(entt::get<TransformComponent, VelocityComponent>);

    for(auto [entity, collision, transform, velocity] : group.each()) {
        auto surface = voxel_surface::UNKNOWN;
        auto vertical_move = vgrid_collide(dimension, 1, collision, transform, velocity, surface);

        if(dimension->entities.any_of<GravityComponent>(entity)) {
            if(vertical_move == vx::sign<int>(dimension->get_gravity())) {
                dimension->entities.emplace_or_replace<GroundedComponent>(entity, GroundedComponent { surface });
            } else {
                dimension->entities.remove<GroundedComponent>(entity);
            }
        } else {
            // The entity cannot be grounded because the component
            // setup of said entity should not let it comprehend the
            // concept of resting on the ground (it flies around)
            dimension->entities.remove<GroundedComponent>(entity);
        }

        vgrid_collide(dimension, 0, collision, transform, velocity, surface);
        vgrid_collide(dimension, 2, collision, transform, velocity, surface);
    }
}
