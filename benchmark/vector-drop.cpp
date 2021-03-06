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

#include <nonius/nonius_single.h++>

#include "util.hpp"

#include <immer/flex_vector.hpp>
#include <immer/flex_vector_transient.hpp>
#include <immer/heap/gc_heap.hpp>
#include <immer/refcount/no_refcount_policy.hpp>
#include <immer/refcount/unsafe_refcount_policy.hpp>

#if IMMER_BENCHMARK_LIBRRB
extern "C" {
#define restrict __restrict__
#include <rrb.h>
#undef restrict
}
#endif

NONIUS_PARAM(N, std::size_t{1000})

#if IMMER_BENCHMARK_LIBRRB
NONIUS_BENCHMARK("librrb", [] (nonius::chronometer meter)
{
    auto n = meter.param<N>();

    auto v = rrb_create();
    for (auto i = 0u; i < n; ++i)
        v = rrb_push(v, reinterpret_cast<void*>(i));

    measure(meter, [&] {
        for (auto i = 0u; i < n; ++i)
            rrb_slice(v, i, n);
    });
});

NONIUS_BENCHMARK("librrb/F", [] (nonius::chronometer meter)
{
    auto n = meter.param<N>();

    auto v = rrb_create();
    for (auto i = 0u; i < n; ++i) {
        auto f = rrb_push(rrb_create(),
                          reinterpret_cast<void*>(i));
        v = rrb_concat(f, v);
    }

    measure(meter, [&] {
        for (auto i = 0u; i < n; ++i)
            rrb_slice(v, i, n);
    });
})

NONIUS_BENCHMARK("l/librrb", [] (nonius::chronometer meter)
{
    auto n = meter.param<N>();

    auto v = rrb_create();
    for (auto i = 0u; i < n; ++i)
        v = rrb_push(v, reinterpret_cast<void*>(i));

    measure(meter, [&] {
        auto r = v;
        for (auto i = 0u; i < n; ++i)
            r = rrb_slice(r, 1, n);
        return r;
    });
});

NONIUS_BENCHMARK("l/librrb/F", [] (nonius::chronometer meter)
{
    auto n = meter.param<N>();

    auto v = rrb_create();
    for (auto i = 0u; i < n; ++i) {
        auto f = rrb_push(rrb_create(),
                          reinterpret_cast<void*>(i));
        v = rrb_concat(f, v);
    }

    measure(meter, [&] {
        auto r = v;
        for (auto i = 0u; i < n; ++i)
            r = rrb_slice(r, 1, n);
        return r;
    });
})

NONIUS_BENCHMARK("t/librrb", [] (nonius::chronometer meter)
{
    auto n = meter.param<N>();

    auto vv = rrb_create();
    for (auto i = 0u; i < n; ++i)
        vv = rrb_push(vv, reinterpret_cast<void*>(i));

    measure(meter, [&] {
        auto v = rrb_to_transient(vv);
        for (auto i = 0u; i < n; ++i)
            v = transient_rrb_slice(v, 1, n);
    });
});

NONIUS_BENCHMARK("t/librrb/F", [] (nonius::chronometer meter)
{
    auto n = meter.param<N>();

    auto vv = rrb_create();
    for (auto i = 0u; i < n; ++i) {
        auto f = rrb_push(rrb_create(),
                          reinterpret_cast<void*>(i));
        vv = rrb_concat(f, vv);
    }

    measure(meter, [&] {
        auto v = rrb_to_transient(vv);
        for (auto i = 0u; i < n; ++i)
            v = transient_rrb_slice(v, 1, n);
    });
})
#endif

template <typename Vektor,
          typename PushFn=push_back_fn>
auto generic()
{
    return [] (nonius::chronometer meter)
    {
        auto n = meter.param<N>();

        auto v = Vektor{};
        for (auto i = 0u; i < n; ++i)
            v = PushFn{}(std::move(v), i);

        measure(meter, [&] {
            for (auto i = 0u; i < n; ++i)
                v.drop(i);
        });
    };
};

template <typename Vektor,
          typename PushFn=push_back_fn>
auto generic_lin()
{
    return [] (nonius::chronometer meter)
    {
        auto n = meter.param<N>();

        auto v = Vektor{};
        for (auto i = 0u; i < n; ++i)
            v = PushFn{}(std::move(v), i);

        measure(meter, [&] {
            auto r = v;
            for (auto i = 0u; i < n; ++i)
                r = r.drop(1);
            return r;
        });
    };
};

template <typename Vektor,
          typename PushFn=push_back_fn>
auto generic_move()
{
    return [] (nonius::chronometer meter)
    {
        auto n = meter.param<N>();

        auto v = Vektor{};
        for (auto i = 0u; i < n; ++i)
            v = PushFn{}(std::move(v), i);

        measure(meter, [&] {
            auto r = v;
            for (auto i = 0u; i < n; ++i)
                r = std::move(r).drop(1);
            return r;
        });
    };
};

template <typename Vektor,
          typename PushFn=push_back_fn>
auto generic_mut()
{
    return [] (nonius::chronometer meter)
    {
        auto n = meter.param<N>();

        auto vv = Vektor{};
        for (auto i = 0u; i < n; ++i)
            vv = PushFn{}(std::move(vv), i);

        measure(meter, [&] {
            auto v = vv.transient();
            for (auto i = 0u; i < n; ++i)
                v.drop(1);
        });
    };
};

using def_memory    = immer::default_memory_policy;
using gc_memory     = immer::memory_policy<immer::heap_policy<immer::gc_heap>, immer::no_refcount_policy>;
using gcf_memory    = immer::memory_policy<immer::heap_policy<immer::gc_heap>, immer::no_refcount_policy, immer::gc_transience_policy, false>;
using basic_memory  = immer::memory_policy<immer::heap_policy<immer::malloc_heap>, immer::refcount_policy>;
using unsafe_memory = immer::memory_policy<immer::default_heap_policy, immer::unsafe_refcount_policy>;

NONIUS_BENCHMARK("flex/4B", generic<immer::flex_vector<unsigned,def_memory,4>>())
NONIUS_BENCHMARK("flex/5B", generic<immer::flex_vector<unsigned,def_memory,5>>())
NONIUS_BENCHMARK("flex/6B", generic<immer::flex_vector<unsigned,def_memory,6>>())
NONIUS_BENCHMARK("flex/GC", generic<immer::flex_vector<unsigned,gc_memory,5>>())
NONIUS_BENCHMARK("flex/NO", generic<immer::flex_vector<unsigned,basic_memory,5>>())
NONIUS_BENCHMARK("flex/UN", generic<immer::flex_vector<unsigned,unsafe_memory,5>>())
NONIUS_BENCHMARK("flex_s/GC", generic<immer::flex_vector<std::size_t,gc_memory,5>>())

NONIUS_BENCHMARK("flex/F/5B", generic<immer::flex_vector<unsigned,def_memory,5>, push_front_fn>())
NONIUS_BENCHMARK("flex/F/GC", generic<immer::flex_vector<unsigned,gc_memory,5>, push_front_fn>())
NONIUS_BENCHMARK("flex/F/GCF", generic<immer::flex_vector<unsigned,gcf_memory,5>, push_front_fn>())
NONIUS_BENCHMARK("flex_s/F/GC", generic<immer::flex_vector<std::size_t,gc_memory,5>, push_front_fn>())

NONIUS_BENCHMARK("l/flex/5B", generic_lin<immer::flex_vector<unsigned,def_memory,5>>())
NONIUS_BENCHMARK("l/flex/GC", generic_lin<immer::flex_vector<unsigned,gc_memory,5>>())
NONIUS_BENCHMARK("l/flex/NO", generic_lin<immer::flex_vector<unsigned,basic_memory,5>>())
NONIUS_BENCHMARK("l/flex/UN", generic_lin<immer::flex_vector<unsigned,unsafe_memory,5>>())
NONIUS_BENCHMARK("l/flex/F/5B", generic_lin<immer::flex_vector<unsigned,def_memory,5>, push_front_fn>())

NONIUS_BENCHMARK("m/flex/5B", generic_move<immer::flex_vector<unsigned,def_memory,5>>())
NONIUS_BENCHMARK("m/flex/GC", generic_move<immer::flex_vector<unsigned,gc_memory,5>>())
NONIUS_BENCHMARK("m/flex/NO", generic_move<immer::flex_vector<unsigned,basic_memory,5>>())
NONIUS_BENCHMARK("m/flex/UN", generic_move<immer::flex_vector<unsigned,unsafe_memory,5>>())
NONIUS_BENCHMARK("m/flex/F/5B", generic_move<immer::flex_vector<unsigned,def_memory,5>, push_front_fn>())

NONIUS_BENCHMARK("t/flex/5B", generic_mut<immer::flex_vector<unsigned,def_memory,5>>())
NONIUS_BENCHMARK("t/flex/GC", generic_mut<immer::flex_vector<unsigned,gc_memory,5>>())
NONIUS_BENCHMARK("t/flex/NO", generic_mut<immer::flex_vector<unsigned,basic_memory,5>>())
NONIUS_BENCHMARK("t/flex/UN", generic_mut<immer::flex_vector<unsigned,unsafe_memory,5>>())
NONIUS_BENCHMARK("t/flex/F/5B", generic_mut<immer::flex_vector<unsigned,def_memory,5>, push_front_fn>())
