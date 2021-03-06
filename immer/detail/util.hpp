//
// immer - immutable data structures for C++
// Copyright (C) 2016, 2017 Juan Pedro Bolivar Puente
//
// This file is part of immer.
//
// immer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// immer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with immer.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <immer/config.hpp>

#include <cstddef>
#include <new>
#include <type_traits>

namespace immer {
namespace detail {

template <class T, class Size>
void destroy_n(T* p, Size n)
{
    auto e = p + n;
    for (; p != e; ++p)
        p->~T();
}

inline void* check_alloc(void* p)
{
    if (IMMER_UNLIKELY(!p))
        throw std::bad_alloc{};
    return p;
}

struct not_supported_t {};
struct empty_t {};

template <typename T>
inline constexpr auto clz_(T) -> not_supported_t { IMMER_UNREACHABLE; return {}; }
inline constexpr auto clz_(unsigned int x) { return __builtin_clz(x); }
inline constexpr auto clz_(unsigned long x) { return __builtin_clzl(x); }
inline constexpr auto clz_(unsigned long long x) { return __builtin_clzll(x); }

template <typename T>
inline constexpr T log2_aux(T x, T r = 0)
{
    return x <= 1 ? r : log2(x >> 1, r + 1);
}

template <typename T>
inline constexpr auto log2(T x)
    -> std::enable_if_t<!std::is_same<decltype(clz_(x)), not_supported_t>{}, T>
{
    return x == 0 ? 0 : sizeof(std::size_t) * 8 - 1 - clz_(x);
}

template <typename T>
inline constexpr auto log2(T x)
    -> std::enable_if_t<std::is_same<decltype(clz_(x)), not_supported_t>{}, T>
{
    return log2_aux(x);
}

template <bool b, typename F>
auto static_if(F&& f) -> std::enable_if_t<b>
{ std::forward<F>(f)(empty_t{}); }
template <bool b, typename F>
auto static_if(F&& f) -> std::enable_if_t<!b>
{}

template <bool b, typename R=void, typename F1, typename F2>
auto static_if(F1&& f1, F2&& f2) -> std::enable_if_t<b, R>
{ return std::forward<F1>(f1)(empty_t{}); }
template <bool b, typename R=void, typename F1, typename F2>
auto static_if(F1&& f1, F2&& f2) -> std::enable_if_t<!b, R>
{ return std::forward<F2>(f2)(empty_t{}); }

} // namespace detail
} // namespace immer
