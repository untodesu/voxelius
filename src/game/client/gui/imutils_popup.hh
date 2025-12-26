#pragma once

/// Returned by popup::show whenever the popup is opened
/// but there is no user action yet, so the UI should wait
constexpr static int POPUP_WAIT = -1;

namespace imutils
{
int popup(const std::string& title, const std::string& question, const std::string* choices, std::size_t num_choices,
    float font_scale = 1.75f);
} // namespace imutils
