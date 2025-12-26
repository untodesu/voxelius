#include "client/pch.hh"

#include "client/experiments.hh"

#include "shared/world/dimension.hh"
#include "shared/world/item_registry.hh"

#include "shared/game_items.hh"
#include "shared/game_voxels.hh"

#include "client/gui/chat.hh"
#include "client/gui/hotbar.hh"
#include "client/gui/status_lines.hh"

#include "client/io/glfw.hh"

#include "client/world/player_target.hh"

#include "client/globals.hh"
#include "client/session.hh"

static void on_glfw_mouse_button(const GlfwMouseButtonEvent& event)
{
    if(!globals::gui_screen && session::is_ingame()) {
        if((event.action == GLFW_PRESS) && player_target::voxel) {
            if(event.button == GLFW_MOUSE_BUTTON_LEFT) {
                experiments::attack();
                return;
            }

            if(event.button == GLFW_MOUSE_BUTTON_RIGHT) {
                experiments::interact();
                return;
            }
        }
    }
}

void experiments::init(void)
{
    globals::dispatcher.sink<GlfwMouseButtonEvent>().connect<&on_glfw_mouse_button>();
}

void experiments::init_late(void)
{
    hotbar::slots[0] = game_items::cobblestone;
    hotbar::slots[1] = game_items::stone;
    hotbar::slots[2] = game_items::dirt;
    hotbar::slots[3] = game_items::grass;
    hotbar::slots[4] = game_items::oak_leaves;
    hotbar::slots[5] = game_items::oak_planks;
    hotbar::slots[6] = game_items::oak_log;
    hotbar::slots[7] = game_items::glass;
    hotbar::slots[8] = game_items::slime;
}

void experiments::shutdown(void)
{
}

void experiments::update(void)
{
}

void experiments::update_late(void)
{
}

void experiments::attack(void)
{
    globals::dimension->set_voxel(nullptr, player_target::coord);
}

void experiments::interact(void)
{
    auto active_item = hotbar::slots[hotbar::active_slot];

    if(active_item) {
        if(auto place_voxel = active_item->get_place_voxel()) {
            globals::dimension->set_voxel(place_voxel, player_target::coord + player_target::normal);
            return;
        }
    }
}
