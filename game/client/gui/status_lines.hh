#ifndef CLIENT_STATUS_LINES_HH
#define CLIENT_STATUS_LINES_HH 1
#pragma once

namespace gui
{
constexpr static unsigned int STATUS_DEBUG = 0x0000;  // generic debug line
constexpr static unsigned int STATUS_HOTBAR = 0x0001; // hotbar item line
constexpr static unsigned int STATUS_COUNT = 0x0002;
} // namespace gui

namespace gui::status_lines
{
void init(void);
void init_late(void);
void layout(void);
} // namespace gui::status_lines

namespace gui::status_lines
{
void set(unsigned int line, const std::string& text, const ImVec4& color, float fadeout);
void unset(unsigned int line);
} // namespace gui::status_lines

#endif // CLIENT_STATUS_LINES_HH
