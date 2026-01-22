/* ---------------------------------------------------------------------- *\
 * include/genpass/Seed.hpp
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

#ifndef __GENPASS_SEED_HPP__
#define __GENPASS_SEED_HPP__

#include <filesystem>  // for path
#include <memory>      // for unique_ptr
#include <string>      // for string
#include <utility>     // for move

class evp_skey_st;

using EVP_SKEY = evp_skey_st;

namespace genpass {

class Seed {
  using EVP_SKEY_ptr = std::unique_ptr<EVP_SKEY, void (*)(EVP_SKEY *)>;

public:
  Seed(EVP_SKEY_ptr&& key)
    : key(std::move(key))
  { }
  ~Seed() { }

  EVP_SKEY *getKey() const { return key.get(); }

  static Seed fromEncryptedFile(
    const std::filesystem::path& file,
    const std::string& password
  );

private:

  const EVP_SKEY_ptr key;
};

} // namespace genpass

#endif // __GENPASS_SEED_HPP__
