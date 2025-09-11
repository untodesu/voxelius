#include "client/pch.hh"

#include "client/gui/hotbar.hh"

#include "core/io/config_map.hh"

#include "core/resource/resource.hh"

#include "shared/world/item_registry.hh"

#include "client/config/keybind.hh"

#include "client/gui/settings.hh"
#include "client/gui/status_lines.hh"

#include "client/io/glfw.hh"

#include "client/resource/texture_gui.hh"

#include "client/globals.hh"

constexpr static float ITEM_SIZE = 20.0f;
constexpr static float ITEM_PADDING = 2.0f;
constexpr static float SELECTOR_PADDING = 1.0f;
constexpr static float HOTBAR_PADDING = 2.0f;

unsigned int gui::hotbar::active_slot = 0U;
item_id gui::hotbar::slots[HOTBAR_SIZE];

static config::KeyBind hotbar_keys[HOTBAR_SIZE];

static resource_ptr<TextureGUI> hotbar_background;
static resource_ptr<TextureGUI> hotbar_selector;

static ImU32 get_color_alpha(ImGuiCol style_color, float alpha)
{
    const auto& color = ImGui::GetStyleColorVec4(style_color);
    return ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, alpha));
}

static void update_hotbar_item(void)
{
    if(gui::hotbar::slots[gui::hotbar::active_slot] == NULL_ITEM_ID) {
        gui::status_lines::unset(gui::STATUS_HOTBAR);
        return;
    }

    if(auto info = world::item_registry::find(gui::hotbar::slots[gui::hotbar::active_slot])) {
        gui::status_lines::set(gui::STATUS_HOTBAR, info->name, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 5.0f);
        return;
    }
}

static void on_glfw_key(const io::GlfwKeyEvent& event)
{
    if((event.action == GLFW_PRESS) && !globals::gui_screen) {
        for(unsigned int i = 0U; i < HOTBAR_SIZE; ++i) {
            if(hotbar_keys[i].equals(event.key)) {
                gui::hotbar::active_slot = i;
                update_hotbar_item();
                break;
            }
        }
    }
}

static void on_glfw_scroll(const io::GlfwScrollEvent& event)
{
    if(!globals::gui_screen) {
        if(event.dy < 0.0) {
            gui::hotbar::next_slot();
            return;
        }

        if(event.dy > 0.0) {
            gui::hotbar::prev_slot();
            return;
        }
    }
}

void gui::hotbar::init(void)
{
    hotbar_keys[0].set_key(GLFW_KEY_1);
    hotbar_keys[1].set_key(GLFW_KEY_2);
    hotbar_keys[2].set_key(GLFW_KEY_3);
    hotbar_keys[3].set_key(GLFW_KEY_4);
    hotbar_keys[4].set_key(GLFW_KEY_5);
    hotbar_keys[5].set_key(GLFW_KEY_6);
    hotbar_keys[6].set_key(GLFW_KEY_7);
    hotbar_keys[7].set_key(GLFW_KEY_8);
    hotbar_keys[8].set_key(GLFW_KEY_9);

    globals::client_config.add_value("hotbar.key.0", hotbar_keys[0]);
    globals::client_config.add_value("hotbar.key.1", hotbar_keys[1]);
    globals::client_config.add_value("hotbar.key.3", hotbar_keys[2]);
    globals::client_config.add_value("hotbar.key.4", hotbar_keys[3]);
    globals::client_config.add_value("hotbar.key.5", hotbar_keys[4]);
    globals::client_config.add_value("hotbar.key.6", hotbar_keys[5]);
    globals::client_config.add_value("hotbar.key.7", hotbar_keys[6]);
    globals::client_config.add_value("hotbar.key.8", hotbar_keys[7]);
    globals::client_config.add_value("hotbar.key.9", hotbar_keys[8]);

    settings::add_keybind(10, hotbar_keys[0], settings_location::KEYBOARD_GAMEPLAY, "hotbar.slot.0");
    settings::add_keybind(11, hotbar_keys[1], settings_location::KEYBOARD_GAMEPLAY, "hotbar.slot.1");
    settings::add_keybind(12, hotbar_keys[2], settings_location::KEYBOARD_GAMEPLAY, "hotbar.slot.2");
    settings::add_keybind(13, hotbar_keys[3], settings_location::KEYBOARD_GAMEPLAY, "hotbar.slot.3");
    settings::add_keybind(14, hotbar_keys[4], settings_location::KEYBOARD_GAMEPLAY, "hotbar.slot.4");
    settings::add_keybind(15, hotbar_keys[5], settings_location::KEYBOARD_GAMEPLAY, "hotbar.slot.5");
    settings::add_keybind(16, hotbar_keys[6], settings_location::KEYBOARD_GAMEPLAY, "hotbar.slot.6");
    settings::add_keybind(17, hotbar_keys[7], settings_location::KEYBOARD_GAMEPLAY, "hotbar.slot.7");
    settings::add_keybind(18, hotbar_keys[8], settings_location::KEYBOARD_GAMEPLAY, "hotbar.slot.8");

    hotbar_background = resource::load<TextureGUI>("textures/gui/hud_hotbar.png", TEXTURE_GUI_LOAD_CLAMP_S | TEXTURE_GUI_LOAD_CLAMP_T);
    hotbar_selector = resource::load<TextureGUI>("textures/gui/hud_selector.png", TEXTURE_GUI_LOAD_CLAMP_S | TEXTURE_GUI_LOAD_CLAMP_T);

    globals::dispatcher.sink<io::GlfwKeyEvent>().connect<&on_glfw_key>();
    globals::dispatcher.sink<io::GlfwScrollEvent>().connect<&on_glfw_scroll>();
}

void gui::hotbar::shutdown(void)
{
    hotbar_background = nullptr;
    hotbar_selector = nullptr;
}

void gui::hotbar::layout(void)
{
    auto& style = ImGui::GetStyle();

    auto item_size = ITEM_SIZE * globals::gui_scale;
    auto hotbar_width = HOTBAR_SIZE * item_size;
    auto hotbar_padding = HOTBAR_PADDING * globals::gui_scale;

    auto viewport = ImGui::GetMainViewport();
    auto draw_list = ImGui::GetForegroundDrawList();

    // Draw the hotbar background image
    auto background_start = ImVec2(0.5f * viewport->Size.x - 0.5f * hotbar_width, viewport->Size.y - item_size - hotbar_padding);
    auto background_end = ImVec2(background_start.x + hotbar_width, background_start.y + item_size);
    draw_list->AddImage(hotbar_background->handle, background_start, background_end);

    // Draw the hotbar selector image
    auto selector_padding_a = SELECTOR_PADDING * globals::gui_scale;
    auto selector_padding_b = SELECTOR_PADDING * globals::gui_scale * 2.0f;
    auto selector_start = ImVec2(background_start.x + gui::hotbar::active_slot * item_size - selector_padding_a,
        background_start.y - selector_padding_a);
    auto selector_end = ImVec2(selector_start.x + item_size + selector_padding_b, selector_start.y + item_size + selector_padding_b);
    draw_list->AddImage(hotbar_selector->handle, selector_start, selector_end);

    // Figure out item texture padding values
    auto item_padding_a = ITEM_PADDING * globals::gui_scale;
    auto item_padding_b = ITEM_PADDING * globals::gui_scale * 2.0f;

    // Draw individual item textures in the hotbar
    for(std::size_t i = 0; i < HOTBAR_SIZE; ++i) {
        const auto info = world::item_registry::find(gui::hotbar::slots[i]);

        if((info == nullptr) || (info->cached_texture == nullptr)) {
            // There's either no item in the slot
            // or the item doesn't have a texture
            continue;
        }

        const auto item_start = ImVec2(background_start.x + i * item_size + item_padding_a, background_start.y + item_padding_a);
        const auto item_end = ImVec2(item_start.x + item_size - item_padding_b, item_start.y + item_size - item_padding_b);
        draw_list->AddImage(info->cached_texture->handle, item_start, item_end);
    }
}

void gui::hotbar::next_slot(void)
{
    gui::hotbar::active_slot += 1U;
    gui::hotbar::active_slot %= HOTBAR_SIZE;
    update_hotbar_item();
}

void gui::hotbar::prev_slot(void)
{
    gui::hotbar::active_slot += HOTBAR_SIZE - 1U;
    gui::hotbar::active_slot %= HOTBAR_SIZE;
    update_hotbar_item();
}
