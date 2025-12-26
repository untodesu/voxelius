// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: concepts.hh
// Description: C++20 concepts that should be in standard library but are not

#ifndef CORE_MATH_CONCEPTS_HH
#define CORE_MATH_CONCEPTS_HH
#pragma once

namespace math
{
template<typename type>
concept arithmetic = std::is_arithmetic_v<type>;
template<typename type>
concept signed_arithmetic = std::is_arithmetic_v<type> && std::is_signed_v<type>;
template<typename type>
concept unsigned_arithmetic = std::is_arithmetic_v<type> && std::is_unsigned_v<type>;
} // namespace math

#endif
