/* ---------------------------------------------------------------------- *\
 * src/util/serialize.hpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2025      David Bears <dbear4q@gmail.com>
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


namespace genpass {
template<typename T>
std::size_t serialize(unsigned char *dst, const T src);

template<typename T>
std::size_t deserialize(T& dst, const unsigned char *src)

template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
std::size_t serialize<T>(unsigned char *dst, const T src) {
  for(int i = 0; i < sizeof(T); i++)
    dst[i] = (unsigned char)(src >> (i * 8));
  return sizeof(T);
}

template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
std::size_t deserialize<T>(T& dst, const unsigned char *src) {
  dst = 0;
  for(int i = 0; i < sizeof(T); i++)
    dst |= (T)src[i] << (i * 8);
  return sizeof(T);
}
}
