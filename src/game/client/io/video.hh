// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: video.hh; Created: Tue Dec 30 2025 12:59:09
// Description: Video mode handling

#ifndef CLIENT_IO_VIDEO_HH
#define CLIENT_IO_VIDEO_HH
#pragma once

class VideoMode final {
public:
    constexpr explicit VideoMode(const GLFWvidmode& mode) noexcept;
    constexpr explicit VideoMode(int wide, int tall, int rate = GLFW_DONT_CARE) noexcept;

    constexpr int wide(void) const noexcept;
    constexpr int tall(void) const noexcept;
    constexpr int rate(void) const noexcept;

    constexpr bool operator==(const VideoMode& other) const noexcept;

private:
    int m_wide;
    int m_tall;
    int m_rate;
};

class FramebufferSizeEvent final {
public:
    constexpr explicit FramebufferSizeEvent(int wide, int tall) noexcept;
    constexpr int wide(void) const noexcept;
    constexpr int tall(void) const noexcept;

private:
    int m_wide;
    int m_tall;
};

namespace video
{
void init(void);
void init_late(void);
void shutdown(void);
void update(void);
void update_late(void);
} // namespace video

namespace video
{
void query_current_mode(int& wide, int& tall) noexcept;
void query_current_mode(int& wide, int& tall, bool& fullscreen) noexcept;
const std::vector<VideoMode>& query_fullscreen_modes(void) noexcept;
} // namespace video

namespace video
{
void request_fullscreen(int wide, int tall, int rate) noexcept;
void request_windowed(int wide, int tall) noexcept;
void request_windowed(void) noexcept;
} // namespace video

namespace video
{
void update_window_title(void);
} // namespace video

constexpr VideoMode::VideoMode(int wide, int tall, int rate) noexcept : m_wide(wide), m_tall(tall), m_rate(rate)
{
    // empty
}

constexpr VideoMode::VideoMode(const GLFWvidmode& mode) noexcept : m_wide(mode.width), m_tall(mode.height), m_rate(mode.refreshRate)
{
    // empty
}

constexpr int VideoMode::wide(void) const noexcept
{
    return m_wide;
}

constexpr int VideoMode::tall(void) const noexcept
{
    return m_tall;
}

constexpr int VideoMode::rate(void) const noexcept
{
    return m_rate;
}

constexpr bool VideoMode::operator==(const VideoMode& other) const noexcept
{
    auto result = true;
    result = result && (m_wide == other.m_wide || m_wide == GLFW_DONT_CARE || other.m_wide == GLFW_DONT_CARE);
    result = result && (m_tall == other.m_tall || m_tall == GLFW_DONT_CARE || other.m_tall == GLFW_DONT_CARE);
    result = result && (m_rate == other.m_rate || m_rate == GLFW_DONT_CARE || other.m_rate == GLFW_DONT_CARE);
    return result;
}

constexpr FramebufferSizeEvent::FramebufferSizeEvent(int wide, int tall) noexcept : m_wide(wide), m_tall(tall)
{
    // empty
}

constexpr int FramebufferSizeEvent::wide(void) const noexcept
{
    return m_wide;
}

constexpr int FramebufferSizeEvent::tall(void) const noexcept
{
    return m_tall;
}

#endif
