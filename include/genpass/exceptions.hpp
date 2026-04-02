/* ------------------------------------------------------------------------ *\
 * include/genpass/exceptions.hpp
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

#ifndef __GENPASS_EXCEPTIONS_HPP__
#define __GENPASS_EXCEPTIONS_HPP__

#include <exception>
#include <stdexcept>
#include <string>

namespace genpass {

class ossl_error : public std::exception {
public:
  ossl_error() noexcept;
  virtual ~ossl_error() noexcept { }

  const char *what() const noexcept override { return whatStr.c_str(); }

private:
  std::string whatStr;
};

class WrongKeyException : public std::runtime_error {
public:
  WrongKeyException() : std::runtime_error("wrong key") { }
};

}

#endif // __GENPASS_EXCEPTIONS_HPP__
