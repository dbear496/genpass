# ---------------------------------------------------------------------- *\
# src/CMakeLists.txt
# This file is part of GenPass.
#
# Copyright (C) 2025-2026 David Bears <dbear4q@gmail.com>
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

add_library(genpass
  Genpass.cpp
  Password.cpp
  Seed.cpp
)
target_sources(genpass PUBLIC FILE_SET HEADERS FILES
  Genpass.hpp
  Password.hpp
  Seed.hpp
  util/IndirectIterator.hpp
)
target_sources(genpass PRIVATE
  util/fmt_nlohmann.hpp
  util/ossl_ptr.hpp
  util/serialize.hpp
)

target_compile_features(genpass PUBLIC cxx_std_20)

target_link_libraries(genpass
  PUBLIC
  nlohmann_json::nlohmann_json
  OpenSSL::Crypto
  PRIVATE
  fmt::fmt
)
set_target_properties(genpass PROPERTIES
  OUTPUT_NAME ${LIBNAME}
  EXPORT_NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  SOVERSION ${ABI_VERSION}
)

install(TARGETS genpass
  EXPORT genpass_export
  FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_APPINCLUDEDIR}
)
