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

#include <immer/heap/tags.hpp>

#if IMMER_HAS_LIBGC
#include <gc/gc.h>
#else
#error "Using garbage collection requires libgc"
#endif

#include <cstdlib>

namespace immer {

#ifdef __APPLE__
#define IMMER_GC_REQUIRE_INIT 1
#else
#define IMMER_GC_REQUIRE_INIT 0
#endif

#if IMMER_GC_REQUIRE_INIT

namespace detail {

template <int Dummy=0>
struct gc_initializer
{
    gc_initializer() { GC_init(); }
    static gc_initializer init;
};

template <int D>
gc_initializer<D> gc_initializer<D>::init {};

inline void gc_initializer_guard()
{
    static gc_initializer<> init_ = gc_initializer<>::init;
    (void) init_;
}

} // namespace detail

#define IMMER_GC_INIT_GUARD_ ::immer::detail::gc_initializer_guard()

#else

#define IMMER_GC_INIT_GUARD_

#endif // IMMER_GC_REQUIRE_INIT

/*!
 * Heap that uses a tracing garbage collector.
 *
 * @rst
 *
 * This heap uses the `Boehm's conservative garbage collector`_ under
 * the hood.  This is a tracing garbage collector that automatically
 * reclaims unused memory.  Thus, it is not needed to call
 * ``deallocate()`` in order to release memory.
 *
 * .. admonition:: Dependencies
 *    :class: tip
 *
 *    In order to use this header file, you need to make sure that
 *    Boehm's ``libgc`` is your include path and link to its binary
 *    library.
 *
 * .. caution:: Memory that is allocated with the standard ``malloc``
 *    and ``free`` is not visible to ``libgc`` when it is looking for
 *    references.  This means that if, let's say, you store a
 *    :cpp:class:`immer::vector` using a ``gc_heap`` inside a
 *    ``std::vector`` that uses a standard allocator, the memory of
 *    the former might be released automatically at unexpected times
 *    causing crashes.
 *
 * .. caution:: When using a ``gc_heap`` in combination with immutable
 *    containers, the destructors of the contained objects will never
 *    be called.  It is ok to store containers inside containers as
 *    long as all of them use a ``gc_heap`` consistently, but storing
 *    other kinds of objects with relevant destructors
 *    (e.g. containers with reference counting or other kinds of
 *    *resource handles*) might cause memory leaks and other problems.
 *
 * .. _boehm's conservative garbage collector: https://github.com/ivmai/bdwgc
 *
 * @endrst
 */
class gc_heap
{
public:
    static void* allocate(std::size_t n)
    {
        IMMER_GC_INIT_GUARD_;
        return GC_malloc(n);
    }

    static void* allocate(std::size_t n, norefs_tag)
    {
        IMMER_GC_INIT_GUARD_;
        return GC_malloc_atomic(n);
    }

    static void deallocate(void* data)
    {
        GC_free(data);
    }

    static void deallocate(void* data, norefs_tag)
    {
        GC_free(data);
    }
};

} // namespace immer
