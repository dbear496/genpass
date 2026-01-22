# ---------------------------------------------------------------------- *\
# include/genpass/detail/CMakeLists.txt
# This file is part of GenPass.
#
# Copyright (C) 2026      David Bears <dbear4q@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
# ---------------------------------------------------------------------- */


target_sources(genpass
  PUBLIC FILE_SET HEADERS FILES
  IndirectIterator.hpp
  PRIVATE
  fmt_nlohmann.hpp
  ossl_ptr.hpp
  serialize.hpp
)

target_link_libraries(genpass
  PUBLIC
  PRIVATE
  nlohmann_json::nlohmann_json
  OpenSSL::Crypto
  fmt::fmt
)
