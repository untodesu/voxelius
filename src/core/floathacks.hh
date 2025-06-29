#ifndef CORE_FLOATHACKS_HH
#define CORE_FLOATHACKS_HH 1
#pragma once

namespace floathacks
{
static inline float int32_to_float(const std::int32_t value);
static inline float uint32_to_float(const std::uint32_t value);
static inline std::int32_t float_to_int32(const float value);
static inline std::uint32_t float_to_uint32(const float value);
} // namespace floathacks

static_assert(std::numeric_limits<float>::is_iec559, "Floathacks only works with IEEE 754 compliant floats");
static_assert(sizeof(std::int32_t) == sizeof(float), "Floathacks requires 32-bit integers to match float size");

static inline float floathacks::int32_to_float(const std::int32_t value)
{
    union {
        std::int32_t src;
        float dst;
    } hack;
    hack.src = value;
    return hack.dst;
}

static inline float floathacks::uint32_to_float(const std::uint32_t value)
{
    union {
        std::uint32_t src;
        float dst;
    } hack;
    hack.src = value;
    return hack.dst;
}

static inline std::int32_t floathacks::float_to_int32(const float value)
{
    union {
        float src;
        std::int32_t dst;
    } hack;
    hack.src = value;
    return hack.dst;
}

static inline std::uint32_t floathacks::float_to_uint32(const float value)
{
    union {
        float src;
        std::uint32_t dst;
    } hack;
    hack.src = value;
    return hack.dst;
}

#endif /* CORE_FLOATHACKS_HH */
