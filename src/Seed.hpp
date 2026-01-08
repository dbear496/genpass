/* ---------------------------------------------------------------------- *\
 * src/Seed.hpp
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


namespace genpass {
class Seed {
public:
  Seed(std::unique_ptr<EVP_SKEY>&& key)
    : key(std::move(key))
  { }
  ~Seed() { }

  const EVP_SKEY *getKey() const { return key.get(); }

  static Seed fromEncryptedFile(
    std::filesystem::Path file,
    std::string password
  );

private:

  const std::unique_ptr<EVP_SKEY> key;
}
