// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: crc64.hh
// Description: Client-server synchronized CRC64

#ifndef CORE_MATH_CRC64_HH
#define CORE_MATH_CRC64_HH
#pragma once

namespace math
{
std::uint64_t crc64(const void* buffer, std::size_t size, std::uint64_t combine = UINT64_C(0));
std::uint64_t crc64(const std::vector<std::byte>& buffer, std::uint64_t combine = UINT64_C(0));
std::uint64_t crc64(const std::string& buffer, std::uint64_t combine = UINT64_C(0));
std::uint64_t crc64(std::string_view buffer, std::uint64_t combine = UINT64_C(0));
} // namespace math

#endif
