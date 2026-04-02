/* ------------------------------------------------------------------------ *\
 * include/genpass/Seed.hpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2025-2026 David Bears <dbear4q@gmail.com>
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

#ifndef __GENPASS_SEED_HPP__
#define __GENPASS_SEED_HPP__

#include <filesystem>  // for path
#include <memory>      // for unique_ptr
#include <string>      // for string
#include <utility>     // for move

namespace genpass {

class Seed {
public:

  static const std::size_t SIZE = 32;

  Seed(std::unique_ptr<const unsigned char[]>&& data) : data(std::move(data)) { }
  ~Seed() { }

  const unsigned char *getData() const { return data.get(); }

  void toEncryptedFile(
    const std::filesystem::path& file,
    const std::string& password
  );

  static Seed fromEncryptedFile(
    const std::filesystem::path& file,
    const std::string& password
  );

  static Seed fromPassword(const std::string& password);

private:

  std::unique_ptr<const unsigned char[]> data;

  static std::unique_ptr<unsigned char[]> allocData() {
    return std::make_unique<unsigned char[]>(Seed::SIZE);
  }
};

} // namespace genpass

#endif // __GENPASS_SEED_HPP__
