/* ------------------------------------------------------------------------ *\
 * src/exceptions.cpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2026      David Bears <dbear4q@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
\* ------------------------------------------------------------------------ */

#include "genpass/exceptions.hpp"

#include <fmt/format.h>
#include <openssl/err.h>

namespace genpass {

ossl_error::ossl_error() noexcept : std::exception() {
  unsigned long code;
  const char *library;
  const char *func;
  const char *reason;
  const char *file;
  int line;
  int flags;
  const char *data;
  code = ERR_peek_last_error_all(&file, &line, &func, &data, &flags);
  library = ERR_lib_error_string(code);
  reason = ERR_reason_error_string(code);
  whatStr = fmt::format("error:{:d}:{:s}:{:s}:{:s}:{:s}:{:d}:{:x}:{:s}",
    code, library, func, reason, file, line, flags, data
  );
}

}
