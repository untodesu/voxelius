#include "client/pch.hh"

#include "client/system/player_target.hh"

#include "core/camera.hh"

#include "shared/net/packet_player.hh"
#include "shared/net/protocol.hh"
#include "shared/utils/coord.hh"
#include "shared/utils/world.hh"
#include "shared/world/block_collisions.hh"
#include "shared/world/block_registry.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/world/outline.hh"

physics::Hit player_target::hit;

static void on_mouse_button_event(const SDL_MouseButtonEvent& event)
{
    if(event.down && !globals::gui_screen && std::holds_alternative<physics::BlockHit>(player_target::hit)) {
        const auto& hit = std::get<physics::BlockHit>(player_target::hit);

        if(event.button == SDL_BUTTON_RIGHT) {
            // TODO: hardcoded placed block until an inventory/hotbar system exists
            auto family = block_registry::find_family(Identifier::from_string("builtin:water"));
            auto block = family ? family->default_variant : block_registry::find(Identifier::from_string("builtin:water"));
            utils::block_place(hit, globals::player, block);

            if(globals::peer) {
                PlayerInteractB_Packet packet {};
                packet.bpos = hit.block_pos;
                packet.expected = hit.id;
                packet.face = hit.face;
                packet.normal = hit.normal;
                packet.point = hit.point;
                protocol::send(packet, globals::peer);
            }
        }
        else if(event.button == SDL_BUTTON_LEFT) {
            utils::block_break(hit, globals::player);

            if(globals::peer) {
                PlayerAttackB_Packet packet {};
                packet.bpos = hit.block_pos;
                packet.expected = hit.id;
                protocol::send(packet, globals::peer);
            }
        }
    }
}

void player_target::init(void)
{
    hit = std::monostate();

    globals::dispatcher.sink<SDL_MouseButtonEvent>().connect<&on_mouse_button_event>();
}

void player_target::update(void)
{
    physics::Ray ray {};
    ray.start_chunk = camera::chunk;
    ray.start = camera::instance.position();
    ray.direction = camera::forward;
    ray.max_distance = 16.0f;

    hit = physics::raycast(ray, physics::BLOCK_FILTER_ALL, physics::ENTITY_FILTER_ALL);
}

void player_target::render(void)
{
    if(auto block_hit = std::get_if<physics::BlockHit>(&hit)) {
        if(auto bcoll = block_collisions::find(block_hit->id)) {
            Eigen::AlignedBox3f aabb = bcoll->bounds;
            aabb.translate(block_hit->local_pos.cast<float>());

            glEnable(GL_DEPTH_TEST);

            outline::prepare();
            outline::cube(block_hit->chunk_pos, aabb.min(), aabb.sizes(), 2.0f, Eigen::Vector4f(0.00f, 0.00f, 0.00f, 1.00f));
        }
    }
}
