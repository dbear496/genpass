/* ---------------------------------------------------------------------- *\
 * src/Genpass.hpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2025-2026 David Bears <dbear4q@gmail.com>
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

#ifndef __GENPASS_GENPASS_HPP__
#define __GENPASS_GENPASS_HPP__

#include <nlohmann/json.hpp>      // for basic_json
#include <nlohmann/json_fwd.hpp>  // for json
#include <functional>             // for function
#include <map>                    // for operator==
#include <memory>                 // for unique_ptr
#include <string>                 // for string, hash, basic_string
#include <unordered_map>          // for unordered_map

#include "Password.hpp"           // for Password

namespace genpass {

class Genpass {
  using Loader =
    std::function<std::unique_ptr<Password>(const nlohmann::json&)>;

public:
  Password& getPassword(const std::string& id) const;
  void addPassword(std::unique_ptr<Password>&& password);
  void removePassword(const std::string& id);

  void registerLoader(const std::string& name, Loader loader);

  template<typename I>
  void readConfig(I&& in) {
    readConfig(nlohmann::json::parse(in));
  }
  std::string writeConfig() const;

private:
  std::unordered_map<std::string, Loader> loaders;
  std::unordered_map<std::string, std::unique_ptr<Password>> passwords;
};

} // namespace genpass

#endif // __GENPASS_GENPASS_HPP__
