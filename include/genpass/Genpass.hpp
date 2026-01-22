/* ---------------------------------------------------------------------- *\
 * include/genpass/Genpass.hpp
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

#include "genpass/Password.hpp"           // for Password
#include "genpass/detail/IndirectIterator.hpp"

namespace genpass {

class Genpass {
private:
  std::unordered_map<std::string, std::unique_ptr<Password>> passwords;
  std::unordered_map<std::string, std::function<Password *()>> algorithms;

public:
  using PasswordIterator = detail::IndirectIterator<
    Password, decltype(passwords)::iterator,
    [](const auto& it) -> Password * { return it->second.get(); }
  >;
  using ConstPasswordIterator = detail::IndirectIterator<
    const Password, decltype(passwords)::const_iterator,
    [](const auto& it) -> const Password * { return it->second.get(); }
  >;

  Genpass();
  ~Genpass();

  Password& addPassword(std::unique_ptr<Password>&& password);
  Password& newPassword(const std::string& algorithm, const std::string& id);
  Password& getPassword(const std::string& id) const;
  void removePassword(const std::string& id);

  PasswordIterator passwords_begin() { return passwords.begin(); }
  PasswordIterator passwords_end() { return passwords.end(); }
  ConstPasswordIterator passwords_cbegin() const { return passwords.cbegin(); }
  ConstPasswordIterator passwords_cend() const { return passwords.cend(); }

  void updateId(const std::string& oldId);
  void updateAllIds();

  template<typename I>
  void deserialize(I&& in) {
    deserialize(nlohmann::json::parse(in));
  }
  nlohmann::json serialize() const;
  void clearPasswords();

  void registerAlgorithm(const std::string& name,
    std::function<Password *()> constructor);
};

} // namespace genpass

#endif // __GENPASS_GENPASS_HPP__
