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

#include <immer/heap/heap_policy.hpp>
#include <immer/refcount/refcount_policy.hpp>
#include <immer/refcount/unsafe_refcount_policy.hpp>
#include <immer/refcount/no_refcount_policy.hpp>
#include <immer/transience/no_transience_policy.hpp>
#include <immer/transience/gc_transience_policy.hpp>
#include <type_traits>

namespace immer {

/*!
 * Metafunction that returns the best *transience policy* to use for a
 * given *refcount policy*.
 */
template <typename RefcountPolicy>
struct get_transience_policy
    : std::conditional<std::is_same<RefcountPolicy,
                                    no_refcount_policy>::value,
                       gc_transience_policy,
                       no_transience_policy>
{};

template <typename T>
using get_transience_policy_t = typename get_transience_policy<T>::type;

/*!
 * Metafunction that returns wether to *prefer fewer bigger objects*
 * to use for a given *heap policy*.
 */
template <typename HeapPolicy>
struct get_prefer_fewer_bigger_objects
    : std::integral_constant<bool,
                             std::is_same<
                                 HeapPolicy,
                                 heap_policy<malloc_heap>>::value>
{};

template <typename T>
constexpr auto get_prefer_fewer_bigger_objects_v =
    get_prefer_fewer_bigger_objects<T>::value;

/*!
 * Metafunction that returns wether to use *transient R-Values*
 * for a given *refcount policy*.
 */
template <typename RefcountPolicy>
struct get_use_transient_rvalues
    : std::integral_constant<bool,
                             !std::is_same<
                                 RefcountPolicy,
                                 no_refcount_policy>::value>
{};

template <typename T>
constexpr auto get_use_transient_rvalues_v = get_use_transient_rvalues<T>::value;

/*!
 * This is a default implementation of a *memory policy*.  A memory
 * policy is just a bag of other policies plus some flags with hints
 * to the user about the best way to use these strategies.
 *
 * @tparam HeapPolicy A *heap policy*, for example, @ref heap_policy.
 * @tparam RefcountPolicy A *reference counting policy*, for example,
 *         @ref refcount_policy.
 * @tparam TransiencePolicy A *transience policy*, for example,
 *         @ref no_transience_policy.
 * @tparam PreferFewerBiggerObjects Boolean flag indicating whether
 *         the user should prefer to allocate memory in bigger chungs
 *         --e.g. by putting various objects in the same memory
 *         region-- or not.
 * @tparam UseTransientRValues Boolean flag indicating whether
 *         immutable containers should try to modify contents in-place
 *         when manipulating an r-value reference.
 */
template <typename HeapPolicy,
          typename RefcountPolicy,
          typename TransiencePolicy     = get_transience_policy_t<RefcountPolicy>,
          bool PreferFewerBiggerObjects = get_prefer_fewer_bigger_objects_v<HeapPolicy>,
          bool UseTransientRValues      = get_use_transient_rvalues_v<RefcountPolicy>>
struct memory_policy
{
    using heap       = HeapPolicy;
    using refcount   = RefcountPolicy;
    using transience = TransiencePolicy;

    static constexpr bool prefer_fewer_bigger_objects =
        PreferFewerBiggerObjects;

    static constexpr bool use_transient_rvalues =
        UseTransientRValues;

    using transience_t = typename transience::template apply<heap>::type;
};

/*!
 * The default *heap policy* just uses the standard heap with a
 * @ref free_list_heap_policy.  If `IMMER_NO_FREE_LIST` is defined to `1`
 * then it just uses the standard heap.
 */
#if IMMER_NO_FREE_LIST
using default_heap_policy = heap_policy<malloc_heap>;
#else
#if IMMER_NO_THREAD_SAFETY
using default_heap_policy = unsafe_free_list_heap_policy<malloc_heap>;
#else
using default_heap_policy = free_list_heap_policy<malloc_heap>;
#endif
#endif

/*!
 * By default we use thread safe reference counting.
 */
#if IMMER_NO_THREAD_SAFETY
using default_refcount_policy = unsafe_refcount_policy;
#else
using default_refcount_policy = refcount_policy;
#endif

/*!
 * The default memory policy.
 */
using default_memory_policy = memory_policy<
    default_heap_policy,
    default_refcount_policy>;

} // namespace immer
