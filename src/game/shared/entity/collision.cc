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

static int vgrid_collide(const Dimension* dimension, int d, Collision& collision, Transform& transform, Velocity& velocity,
    VoxelMaterial& touch_surface, bool enable_snapping_to_grid)
{
    auto movespeed = globals::fixed_frametime * velocity.value[d];
    auto movesign = math::sign<int>(movespeed);

    auto& ref_aabb = collision.aabb;
    auto ref_center = 0.5f * ref_aabb.min + 0.5f * ref_aabb.max;
    auto ref_halfsize = 0.5f * ref_aabb.max - 0.5f * ref_aabb.min;

    auto curr_aabb = ref_aabb.push(transform.local);

    math::AABBf next_aabb(curr_aabb);
    next_aabb.min[d] += movespeed;
    next_aabb.max[d] += movespeed;

    auto next_center = 0.5f * next_aabb.min + 0.5f * next_aabb.max;

    auto csg_aabb = curr_aabb.combine(next_aabb);

    local_pos lpos_min;
    lpos_min.x = static_cast<local_pos::value_type>(glm::floor(csg_aabb.min.x));
    lpos_min.y = static_cast<local_pos::value_type>(glm::floor(csg_aabb.min.y));
    lpos_min.z = static_cast<local_pos::value_type>(glm::floor(csg_aabb.min.z));

    local_pos lpos_max;
    lpos_max.x = static_cast<local_pos::value_type>(glm::ceil(csg_aabb.max.x));
    lpos_max.y = static_cast<local_pos::value_type>(glm::ceil(csg_aabb.max.y));
    lpos_max.z = static_cast<local_pos::value_type>(glm::ceil(csg_aabb.max.z));

    auto u = (d + 1) % 3;
    auto v = (d + 2) % 3;

    local_pos::value_type ddir;
    local_pos::value_type dmin;
    local_pos::value_type dmax;

    if(movespeed < 0.0f) {
        ddir = local_pos::value_type(+1);
        dmin = lpos_min[d];
        dmax = lpos_max[d];
    }
    else {
        ddir = local_pos::value_type(-1);
        dmin = lpos_max[d];
        dmax = lpos_min[d];
    }

    auto closest_dist = std::numeric_limits<float>::infinity();
    auto closest_touch = VTOUCH_NONE;
    auto closest_surface = VMAT_UNKNOWN;
    auto closest_tvalues = ZERO_VEC3<float>;
    math::AABBf closest_vbox;

    for(auto i = dmin; i != dmax; i += ddir) {
        for(auto j = lpos_min[u]; j < lpos_max[u]; ++j) {
            for(auto k = lpos_min[v]; k < lpos_max[v]; ++k) {
                local_pos lpos;
                lpos[d] = i;
                lpos[u] = j;
                lpos[v] = k;

                auto vpos = coord::to_voxel(transform.chunk, lpos);
                auto voxel = dimension->get_voxel(vpos);

                if(voxel == nullptr || voxel->is_touch_type<VTOUCH_NONE>()) {
                    continue; // non-collidable
                }

                auto vbox = voxel->get_collision().push(lpos);
                auto vbox_center = 0.5f * vbox.min + 0.5f * vbox.max;

                if(!csg_aabb.intersect(vbox)) {
                    continue; // no intersection
                }

                auto distance = glm::abs(vbox_center[d] - next_center[d]);

                if(distance < closest_dist) {
                    closest_dist = distance;
                    closest_touch = voxel->get_touch_type();
                    closest_surface = voxel->get_surface_material();
                    closest_tvalues = voxel->get_touch_values();
                    closest_vbox = vbox;
                }
            }
        }
    }

    if(std::isfinite(closest_dist)) {
        auto snap_to_closest_vbox = false;

        if(closest_touch == VTOUCH_BOUNCE) {
            auto threshold = 2.0f * static_cast<float>(globals::fixed_frametime);
            auto travel_distance = glm::abs(curr_aabb.min[d] - next_aabb.min[d]);

            if(travel_distance > threshold) {
                velocity.value[d] = -velocity.value[d] * closest_tvalues[d];
            }
            else {
                velocity.value[d] = 0.0f;
            }

            snap_to_closest_vbox = true;
        }
        else if(closest_touch == VTOUCH_SINK) {
            constexpr auto threshold = 0.01f;

            if(velocity.value[d] < threshold) {
                velocity.value[d] = 0.0f;
            }
            else {
                velocity.value[d] *= closest_tvalues[d];
            }
        }
        else {
            velocity.value[d] = 0.0f;
            snap_to_closest_vbox = true;
        }

        if(snap_to_closest_vbox && enable_snapping_to_grid) {
            auto vbox_center = 0.5f * closest_vbox.min[d] + 0.5f * closest_vbox.max[d];
            auto vbox_halfsize = 0.5f * closest_vbox.max[d] - 0.5f * closest_vbox.min[d];

            if(movesign < 0) {
                transform.local[d] = vbox_center + vbox_halfsize + ref_halfsize[d] - ref_center[d] + 0.01f * globals::fixed_frametime;
            }
            else {
                transform.local[d] = vbox_center - vbox_halfsize - ref_halfsize[d] - ref_center[d] - 0.01f * globals::fixed_frametime;
            }
        }

        touch_surface = closest_surface;

        return movesign;
    }

    touch_surface = VMAT_UNKNOWN;
    return 0;
}

void Collision::fixed_update(Dimension* dimension)
{
    auto group = dimension->entities.group<Collision>(entt::get<Transform, Velocity>);

    for(auto [entity, collision, transform, velocity] : group.each()) {
        auto surface = VMAT_UNKNOWN;
        auto vertical_move = vgrid_collide(dimension, 1, collision, transform, velocity, surface, true);

        if(dimension->entities.any_of<Gravity>(entity)) {
            if(vertical_move == math::sign<int>(dimension->get_gravity())) {
                spdlog::info("grounded");
                dimension->entities.emplace_or_replace<Grounded>(entity, Grounded { surface });
            }
            else {
                dimension->entities.remove<Grounded>(entity);
            }
        }
        else {
            // The entity cannot be grounded because the component
            // setup of said entity should not let it comprehend the
            // concept of resting on the ground (it flies around)
            dimension->entities.remove<Grounded>(entity);
        }

        vgrid_collide(dimension, 0, collision, transform, velocity, surface, false);
        vgrid_collide(dimension, 2, collision, transform, velocity, surface, false);
    }
}
