#include "client/pch.hh"

#include "client/experiments.hh"

#include "shared/dimension.hh"
#include "shared/game_items.hh"
#include "shared/game_voxels.hh"
#include "shared/item_registry.hh"

#include "client/chat.hh"
#include "client/glfw.hh"
#include "client/globals.hh"
#include "client/hotbar.hh"
#include "client/player_target.hh"
#include "client/session.hh"
#include "client/status_lines.hh"

static void on_glfw_mouse_button(const GlfwMouseButtonEvent& event)
{
    if(!globals::gui_screen && session::is_ingame()) {
        if((event.action == GLFW_PRESS) && (player_target::voxel != NULL_VOXEL_ID)) {
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

void experiments::deinit(void)
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
    globals::dimension->set_voxel(NULL_VOXEL_ID, player_target::coord);
}

void experiments::interact(void)
{
    if(auto info = item_registry::find(hotbar::slots[hotbar::active_slot])) {
        if(info->place_voxel != NULL_VOXEL_ID) {
            globals::dimension->set_voxel(info->place_voxel, player_target::coord + player_target::normal);
        }
    }
}
