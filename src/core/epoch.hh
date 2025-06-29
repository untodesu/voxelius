#ifndef CORE_EPOCH_HH
#define CORE_EPOCH_HH 1
#pragma once

namespace epoch
{
std::uint64_t seconds(void);
std::uint64_t milliseconds(void);
std::uint64_t microseconds(void);
} // namespace epoch

namespace epoch
{
std::int64_t signed_seconds(void);
std::int64_t signed_milliseconds(void);
std::int64_t signed_microseconds(void);
} // namespace epoch

#endif // CORE_EPOCH_HH
