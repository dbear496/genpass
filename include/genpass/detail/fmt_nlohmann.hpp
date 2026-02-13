/* ---------------------------------------------------------------------- *\
 * src/detail/fmt_nlohmann.hpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2026      David Bears <dbear4q@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
\* ---------------------------------------------------------------------- */

#ifndef __GENPASS_UTIL_FMT_NLOHMANN_HPP__
#define __GENPASS_UTIL_FMT_NLOHMANN_HPP__

#ifndef __GENPASS_PRIVATE__
// IWYU pragma: private
#endif
// IWYU pragma: always_keep

#include <fmt/ostream.h>
#include <nlohmann/json.hpp>

template <> struct fmt::formatter<nlohmann::json> : ostream_formatter {};

#endif // __GENPASS_UTIL_FMT_NLOHMANN_HPP__
