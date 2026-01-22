/* ---------------------------------------------------------------------- *\
 * include/genpass/detail/IndirectIterator.hpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2026      David Bears <dbear4q@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
\* ---------------------------------------------------------------------- */

#ifndef __GENPASS_UTIL_INDIRECTITERATOR_HPP__
#define __GENPASS_UTIL_INDIRECTITERATOR_HPP__

#include <cstddef>

namespace genpass::detail {

template<typename T, typename I, T *(getter)(const I&)>
class IndirectIterator {
public:
  IndirectIterator(const I& it) : it(it) { }
  IndirectIterator(const IndirectIterator<T, I, getter>& it) = default;
  friend auto operator<=>(const IndirectIterator&, const IndirectIterator&) = default;
  IndirectIterator& operator++() { ++it; return *this; }
  IndirectIterator& operator--() { --it; return *this; }
  IndirectIterator operator++(int) { return it++; }
  IndirectIterator operator--(int) { return it--; }
  IndirectIterator& operator+=(std::ptrdiff_t n) { it += n; return *this; }
  IndirectIterator& operator-=(std::ptrdiff_t n) { it -= n; return *this; }
  IndirectIterator operator+(std::ptrdiff_t n) { return it + n; }
  friend IndirectIterator operator+(std::ptrdiff_t n, const IndirectIterator& it) { return it + n; }
  IndirectIterator operator-(std::ptrdiff_t n) { return it - n; }
  IndirectIterator operator-(const IndirectIterator& o) { return it - o.it; }
  T& operator[](std::ptrdiff_t n) { return *operator+(n); }
  T& operator*() const { return *getter(it); }
  T *operator->() const { return getter(it); }
private:
  I it;
};

} // namespace genpass::detail

#endif // __GENPASS_UTIL_INDIRECTITERATOR_HPP__
