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
  VERSION 2.0.0
  LANGUAGES CXX
  DESCRIPTION "A unique password manager that doesn't store passwords."
  HOMEPAGE_URL "https://github.com/dbear496/genpass"
)
set(ABI_VERSION 0)
set(PROJECT_NAME_PRETTY "GenPass")


### dependencies
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/mk/cmake)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
find_package(nlohmann_json 3.12.0 REQUIRED)
find_package(OpenSSL 3.6.0 COMPONENTS crypto REQUIRED)
find_package(fmt 12.0 REQUIRED)


### build configurations
set(BUILD_TYPES CMake Release MinSizeRel RelWithDebInfo BetaTest Debug DebugOpt)
set(GENPASS_BUILD_TYPE "CMAKE" CACHE STRING "GenPass build type")
set_property(CACHE GENPASS_BUILD_TYPE PROPERTY STRINGS ${BUILD_TYPES})
string(TOUPPER "${GENPASS_BUILD_TYPE}" BUILD_TYPE_UPPER)
if(BUILD_TYPE_UPPER STREQUAL "CMAKE")
  string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
endif()
set(GENPASS_CXX_FLAGS_RELEASE        -O3 -w -DNDEBUG)
set(GENPASS_CXX_FLAGS_MINSIZEREL     -Os -w -DNDEBUG)
set(GENPASS_CXX_FLAGS_RELWITHDEBINFO -O3 -g -DNDEBUG)
set(GENPASS_CXX_FLAGS_BETATEST       -O3 -g -w)
set(GENPASS_CXX_FLAGS_DEBUG          -O0 -g -DDEBUG)
set(GENPASS_CXX_FLAGS_DEBUGOPT       -O2 -g -DDEBUG)
foreach(BType ${BUILD_TYPES})
	string(TOUPPER ${BType} BTYPE)
	set(GENPASS_CXX_FLAGS_${BTYPE} ${GENPASS_CXX_FLAGS_${BTYPE}}
		CACHE STRING "Flags used by the CXX compiler for ${BTYPE} builds.")
	set(GENPASS_C_FLAGS_${BCONFIG} ${GENPASS_CXX_FLAGS_${BTYPE}}
		CACHE STRING "Flags used by the C compiler for ${BTYPE} builds.")
endforeach()
add_compile_options(${GENPASS_CXX_FLAGS_${BUILD_TYPE_UPPER}})


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
message(STATUS "${PROJECT_NAME_PRETTY} version: ${FULL_PROJECT_VERSION}")


### compilation options
add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.")
if(DEFINED GENPASS_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ${GENPASS_SHARED_LIBS})
endif()


### install dirs
include(GNUInstallDirs)
set(CMAKE_INSTALL_APPDATADIR
  ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME})
set(CMAKE_INSTALL_FULL_APPDATADIR
  ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME})
set(CMAKE_INSTALL_APPINCLUDEDIR
  ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME}-${PROJECT_VERSION_MAJOR})
set(CMAKE_INSTALL_FULL_APPINCLUDEDIR
  ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${CMAKE_PROJECT_NAME}-${PROJECT_VERSION_MAJOR})


### include what you use
find_program(INCLUDE_WHAT_YOU_USE NAMES include-what-you-use)
if(NOT CMAKE_CROSSCOMPILING AND INCLUDE_WHAT_YOU_USE AND
	NOT GENPASS_CXX_FLAGS_${BUILD_TYPE_UPPER} MATCHES "(^| )-w( |$)"
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
if(GENPASS_CXX_FLAGS_${BUILD_TYPE_UPPER} MATCHES "(^| )-g( |$)")
	set(CPACK_STRIP_FILES FALSE)
else()
	set(CPACK_STRIP_FILES TRUE)
endif()
include(CPack)
