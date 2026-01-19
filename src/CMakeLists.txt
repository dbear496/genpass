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

add_subdirectory(util)

set(LIBNAME ${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}) # genpass-2

add_library(${LIBNAME}
  Genpass.cpp
  Password.cpp
  Seed.cpp
)
target_sources(${LIBNAME} PUBLIC FILE_SET HEADERS FILES
  Genpass.hpp
  Password.hpp
  Seed.hpp
)


target_link_libraries(${LIBNAME}
  PUBLIC
  OpenSSL::Crypto
  PRIVATE
  misc-utils
  nlohmann_json::nlohmann_json
  fmt::fmt
)
target_include_directories(${LIBNAME} PUBLIC .)
target_compile_features(${LIBNAME} PUBLIC cxx_std_20)
set_target_properties(${LIBNAME} PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${ABI_VERSION}
)

install(TARGETS ${LIBNAME}
  FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_APPINCLUDEDIR}
)
