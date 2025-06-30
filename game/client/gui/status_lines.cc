#include "client/pch.hh"

#include "client/gui/status_lines.hh"

#include "client/gui/imdraw_ext.hh"

#include "client/globals.hh"

static float line_offsets[gui::STATUS_COUNT];
static ImFont* line_fonts[gui::STATUS_COUNT];

static ImVec4 line_text_colors[gui::STATUS_COUNT];
static ImVec4 line_shadow_colors[gui::STATUS_COUNT];
static std::string line_strings[gui::STATUS_COUNT];
static std::uint64_t line_spawns[gui::STATUS_COUNT];
static float line_fadeouts[gui::STATUS_COUNT];

void gui::status_lines::init(void)
{
    for(unsigned int i = 0U; i < STATUS_COUNT; ++i) {
        line_text_colors[i] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        line_shadow_colors[i] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        line_strings[i] = std::string();
        line_spawns[i] = UINT64_MAX;
        line_fadeouts[i] = 0.0f;
    }
}

void gui::status_lines::init_late(void)
{
    line_offsets[STATUS_DEBUG] = 64.0f;
    line_offsets[STATUS_HOTBAR] = 40.0f;
}

void gui::status_lines::layout(void)
{
    line_fonts[STATUS_DEBUG] = globals::font_debug;
    line_fonts[STATUS_HOTBAR] = globals::font_chat;

    auto viewport = ImGui::GetMainViewport();
    auto draw_list = ImGui::GetForegroundDrawList();

    for(unsigned int i = 0U; i < STATUS_COUNT; ++i) {
        auto offset = line_offsets[i] * globals::gui_scale;
        auto& text = line_strings[i];
        auto* font = line_fonts[i];

        auto size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, text.c_str(), text.c_str() + text.size());
        auto pos = ImVec2(0.5f * (viewport->Size.x - size.x), viewport->Size.y - offset);

        auto spawn = line_spawns[i];
        auto fadeout = line_fadeouts[i];
        auto alpha = std::exp(-1.0f * std::pow(1.0e-6f * static_cast<float>(globals::curtime - spawn) / fadeout, 10.0f));

        auto& color = line_text_colors[i];
        auto& shadow = line_shadow_colors[i];
        auto color_U32 = ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, color.w * alpha));
        auto shadow_U32 = ImGui::GetColorU32(ImVec4(shadow.x, shadow.y, shadow.z, color.w * alpha));

        gui::imdraw_ext::text_shadow(text, pos, color_U32, shadow_U32, font, draw_list);
    }
}

void gui::status_lines::set(unsigned int line, const std::string& text, const ImVec4& color, float fadeout)
{
    line_text_colors[line] = ImVec4(color.x, color.y, color.z, color.w);
    line_shadow_colors[line] = ImVec4(color.x * 0.1f, color.y * 0.1f, color.z * 0.1f, color.w);
    line_strings[line] = std::string(text);
    line_spawns[line] = globals::curtime;
    line_fadeouts[line] = fadeout;
}

void gui::status_lines::unset(unsigned int line)
{
    line_text_colors[line] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    line_shadow_colors[line] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    line_strings[line] = std::string();
    line_spawns[line] = UINT64_C(0);
    line_fadeouts[line] = 0.0f;
}
