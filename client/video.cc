#include "client/pch.hh"

#include "client/video.hh"

#include "core/config/map.hh"
#include "core/config/ref.hh"

#include "client/globals.hh"

static Eigen::Vector2i s_last_windowed_size;
static config::Ref<bool> s_enable_vsync { true };
static config::Ref<std::string_view> s_current_mode { "windowed" };
