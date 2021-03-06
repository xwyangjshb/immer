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

#include <immer/flex_vector.hpp>
#include <immer/flex_vector_transient.hpp>
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>

template <typename T>
using test_flex_vector_t = immer::flex_vector<T, immer::default_memory_policy, 3u, 0u>;

template <typename T>
using test_flex_vector_transient_t = typename test_flex_vector_t<T>::transient_type;

template <typename T>
using test_vector_t = immer::vector<T, immer::default_memory_policy, 3u, 0u>;

#define FLEX_VECTOR_T           test_flex_vector_t
#define FLEX_VECTOR_TRANSIENT_T test_flex_vector_transient_t
#define VECTOR_T                test_vector_t

#include "generic.ipp"
