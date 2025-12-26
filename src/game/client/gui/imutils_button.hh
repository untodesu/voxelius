#pragma once

namespace imutils
{
void button(const char* title, const ImVec2& size, void (*callback)(void));
void button(const char* title, void (*callback)(void));
} // namespace imutils

namespace imutils
{
bool selectable_button(const char* label, const ImVec2& size, bool value);
bool toggle_button(const char* label, const ImVec2& size, bool& value);
bool toggle_button(const char* label, bool& value);
} // namespace imutils
