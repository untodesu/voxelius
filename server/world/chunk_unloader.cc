#include "server/pch.hh"

#include "server/world/chunk_unloader.hh"

#include "core/config/ref.hh"

#include "shared/component/transform.hh"
#include "shared/globals.hh"
#include "shared/world/chunk.hh"
#include "shared/world/world.hh"

#include "server/constant.hh"
#include "server/globals.hh"
#include "server/net/sessions.hh"
#include "server/world/chunk_loader.hh"

constexpr static std::size_t UNLOAD_BUDGET = constant::REGION_VOLUME;

static config::Ref<unsigned> s_view_distance { 8 };

void chunk_unloader::init(void)
{
    s_view_distance.bind(globals::server_config, "game.view_distance");
}

void chunk_unloader::fixed_update_late(void)
{
    ZoneScoped;

    auto all_sessions = sessions::all();

    std::vector<ChunkAlignedBox> boxes;
    boxes.reserve(all_sessions.size());

    for(auto& session : all_sessions) {
        if(globals::registry.valid(session.player)) {
            if(auto transform = globals::registry.try_get<Transform>(session.player)) {
                auto radius = static_cast<ChunkPos::value_type>(s_view_distance.value());
                auto min = transform->chunk - ChunkPos::Constant(radius);
                auto max = transform->chunk + ChunkPos::Constant(radius);
                boxes.emplace_back(min, max);
            }
        }
    }

    std::vector<ChunkPos> to_unload;
    std::size_t autosaved = 0;

    auto chunks = world::chunk_registry.view<Chunk_Component>();

    for(const auto [entity, component] : chunks.each()) {
        auto inside_view = false;

        for(const auto& box : boxes) {
            if(box.contains(component.position)) {
                inside_view = true;
                break;
            }
        }

        if(!inside_view) {
            if(to_unload.size() < UNLOAD_BUDGET) {
                to_unload.push_back(component.position);
            }

            continue;
        }
    }

    for(const auto& pos : to_unload) {
        chunk_loader::save(pos);
        world::remove_chunk(pos);
    }
}
