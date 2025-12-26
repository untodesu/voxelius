#include "core/pch.hh"

#include "core/version.hh"

// clang-format off
const unsigned short version::major = 16;
const unsigned short version::minor = 0;
const unsigned short version::patch = 1;
// clang-format on

const std::string_view version::commit = "7c2df3b5";
const std::string_view version::branch = "devel";
const std::string_view version::triplet = "16.0.1";

const std::string_view version::full = "16.0.1-7c2df3b5";
