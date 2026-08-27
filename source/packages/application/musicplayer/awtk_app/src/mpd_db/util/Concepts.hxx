// SPDX-License-Identifier: BSD-2-Clause
// author: Max Kellermann <max.kellermann@gmail.com>
// Modified: C++17 compatibility for GCC 7.x toolchain

#pragma once

#if __cpp_concepts >= 202002L
#include <concepts>

template<typename F, typename T>
concept Disposer = std::invocable<F, T *>;
#else
// C++17 fallback: concept not available, use unconstrained templates
#define MPD_NO_CONCEPTS
#endif
