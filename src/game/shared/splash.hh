// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: splash.hh
// Description: Randomized text messages

#ifndef SHARED_SPLASH_HH
#define SHARED_SPLASH_HH
#pragma once

namespace splash
{
void init_client(void);
void init_server(void);
std::string_view get(void);
} // namespace splash

#endif
