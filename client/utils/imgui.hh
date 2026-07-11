#ifndef A1A0393C_6AA5_417C_85EB_2DE26009419F
#define A1A0393C_6AA5_417C_85EB_2DE26009419F

namespace utils
{
void text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font);
void text_nw(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float size);
void text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap);
void text_wr(ImDrawList* list, std::string_view text, const ImVec2& pos, ImU32 color, ImU32 shadow, ImFont* font, float wrap, float size);
} // namespace utils

namespace utils
{
std::optional<int> popup(const std::string& title, const std::string& question, std::span<const std::string> choices,
    float font_scale = 1.75f);
} // namespace utils

namespace utils
{
void button(const char* title, const ImVec2& size, void (*callback)(void));
void button(const char* title, void (*callback)(void));
} // namespace utils

namespace utils
{
bool selectable_button(const char* label, const ImVec2& size, bool value);
bool toggle_button(const char* label, const ImVec2& size, bool& value);
bool toggle_button(const char* label, bool& value);
} // namespace utils

#endif /* A1A0393C_6AA5_417C_85EB_2DE26009419F */
