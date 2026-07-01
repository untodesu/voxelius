#include "core/pch.hh"

#include "core/utils/epoch.hh"

std::uint64_t utils::epoch_seconds(void)
{
    std::chrono::system_clock::duration elapsed(std::chrono::system_clock::now().time_since_epoch());
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
}

std::uint64_t utils::epoch_milliseconds(void)
{
    std::chrono::system_clock::duration elapsed(std::chrono::system_clock::now().time_since_epoch());
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

std::uint64_t utils::epoch_microseconds(void)
{
    std::chrono::system_clock::duration elapsed(std::chrono::system_clock::now().time_since_epoch());
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
}

std::int64_t utils::signed_epoch_seconds(void)
{
    std::chrono::system_clock::duration elapsed(std::chrono::system_clock::now().time_since_epoch());
    return static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
}

std::int64_t utils::signed_epoch_milliseconds(void)
{
    std::chrono::system_clock::duration elapsed(std::chrono::system_clock::now().time_since_epoch());
    return static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

std::int64_t utils::signed_epoch_microseconds(void)
{
    std::chrono::system_clock::duration elapsed(std::chrono::system_clock::now().time_since_epoch());
    return static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
}
