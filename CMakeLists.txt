# ---------------------------------------------------------------------- *\
# CMakeLists.txt
# This file is part of GenPass.
#
# Copyright (C) 2025      David Bears <dbear4q@gmail.com>
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

cmake_minimum_required(VERSION 4.0)

project(genpass
  VERSION 0.0.0
  LANGUAGES CXX
  DESCRIPTION "A unique password manager that doesn't store passwords."
  HOMEPAGE_URL "https://github.com/dbear496/genpass"
)


### dependencies
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/mk/cmake)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
find_package(Qt6 REQUIRED COMPONENTS Widgets)


### build configurations
set(BUILD_TYPES Release MinSizeRel RelWithDebInfo BetaTest Debug DebugOpt)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${BUILD_TYPES})
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
	message(STATUS "No build type specified, defaulting to ${CMAKE_BUILD_TYPE}.")
endif()
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -w -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -w -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
set(CMAKE_CXX_FLAGS_BETATEST       "-O3 -g -w")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUGOPT       "-O2 -g -DDEBUG")
foreach(BType ${BUILD_TYPES})
	string(TOUPPER ${BType} BTYPE)
	set(CMAKE_CXX_FLAGS_${BTYPE} ${CMAKE_CXX_FLAGS_${BTYPE}}
		CACHE STRING "Flags used by the CXX compiler for ${BTYPE} builds.")
	set(CMAKE_C_FLAGS_${BCONFIG} ${CMAKE_CXX_FLAGS_${BTYPE}}
		CACHE STRING "Flags used by the C compiler for ${BTYPE} builds.")
endforeach()


### compute version
set(GENPASS_VERSION_SUFFIX "" CACHE STRING
	"Custom version suffix to use; mostly useful for downstream packaging."
)
include(GetGitRevisionDescription)
if(DEFINED GENPASS_VERSION_SUFFIX AND NOT (GENPASS_VERSION_SUFFIX STREQUAL ""))
	set(FULL_PROJECT_VERSION "${PROJECT_VERSION}-${GENPASS_VERSION_SUFFIX}")
else()
	git_describe_working_tree(FULL_PROJECT_VERSION --match genpass-*)
	if(FULL_PROJECT_VERSION)
		string(SUBSTRING ${FULL_PROJECT_VERSION} 8 -1 FULL_PROJECT_VERSION)
	else()
		set(FULL_PROJECT_VERSION "${PROJECT_VERSION}-unknown")
	endif()
endif()
set(VERSION_RELEASE "")
set(VERSION_MINSIZEREL "")
set(VERSION_RELWITHDEBINFO "debinfo")
set(VERSION_BETATEST "beta")
set(VERSION_DEBUG "debug")
set(VERSION_DEBUGOPT "debug")
if(VERSION_${BUILD_TYPE_UPPER})
	set(FULL_PROJECT_VERSION ${FULL_PROJECT_VERSION}-${VERSION_${BUILD_TYPE_UPPER}})
elseif(NOT DEFINED VERSION_${BUILD_TYPE_UPPER})
	message(WARNING "unrecognized build type: ${BUILD_TYPE_UPPER}")
endif()
message(STATUS "GenPass version: ${FULL_PROJECT_VERSION}")


### compilation options
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.")


### install dirs
include(GNUInstallDirs)


### include what you use
find_program(INCLUDE_WHAT_YOU_USE NAMES include-what-you-use)
if(NOT CMAKE_CROSSCOMPILING AND INCLUDE_WHAT_YOU_USE AND
	NOT CMAKE_CXX_FLAGS_${BUILD_TYPE_UPPER} MATCHES "(^| )-w( |$)"
)
	set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE}
		-w -Xiwyu --mapping_file=${CMAKE_SOURCE_DIR}/mk/iwyu.map)
	set(CMAKE_C_INCLUDE_WHAT_YOU_USE ${CMAKE_CXX_INCLUDE_WHAT_YOU_USE})
endif()


### subdirectories
add_subdirectory(src)


### packaging
set(CPACK_GENERATOR TXZ)
if(WIN32)
	set(CPACK_GENERATOR ZIP)
endif()
set(CPACK_SOURCE_GENERATOR ${CPACK_GENERATOR})
# set(CPACK_PACKAGE_ICON )
set(CPACK_PACKAGE_CHECKSUM SHA256)
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README.md)
set(CPACK_SOURCE_IGNORE_FILES
  ${CMAKE_BINARY_DIR}/
  ${CMAKE_SOURCE_DIR}/\\\\.git/
  ${CMAKE_SOURCE_DIR}/build.*/
)
set(CPACK_PACKAGE_VERSION ${FULL_PROJECT_VERSION})
if(CMAKE_CXX_FLAGS_${BUILD_TYPE_UPPER} MATCHES "(^| )-g( |$)")
	set(CPACK_STRIP_FILES FALSE)
else()
	set(CPACK_STRIP_FILES TRUE)
endif()
include(CPack)
