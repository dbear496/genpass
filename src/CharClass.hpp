/* ---------------------------------------------------------------------- *\
 * src/CharClass.hpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2025-2026 David Bears <dbear4q@gmail.com>
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

#ifndef __GENPASS_CHARCLASS_HPP__
#define __GENPASS_CHARCLASS_HPP__

namespace genpass {

class CharClass {
public:
  CharClass();
  CharClass(std::initializer_list<char>);
  CharClass(std::string)

  std::unordered_set<char> chars;
  char default_;

public:
  static const CharClass ascii;
  static const CharClass base64;
  static const CharClass alpha;
  static const CharClass lowercase;
  static const CharClass uppercase;
  static const CharClass digit;
  static const CharClass special;
};

} // namespace genpass

#endif // __GENPASS_CHARCLASS_HPP__
