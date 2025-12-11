#pragma once

namespace utils
{
std::uint64_t unix_seconds(void);
std::uint64_t unix_milliseconds(void);
std::uint64_t unix_microseconds(void);
} // namespace utils

namespace utils
{
std::int64_t signed_unix_seconds(void);
std::int64_t signed_unix_milliseconds(void);
std::int64_t signed_unix_microseconds(void);
} // namespace utils
