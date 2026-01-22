/* ---------------------------------------------------------------------- *\
 * src/Genpass.cpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2026      David Bears <dbear4q@gmail.com>
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

#include "genpass/Genpass.hpp"

#include <fmt/base.h>    // for println
#include <fmt/format.h>  // for native_formatter::format
#include <stdio.h>       // for stderr
#include <utility>       // for move, pair

#include "genpass/detail/fmt_nlohmann.hpp"
#include "genpass/Password.hpp"  // for Password

namespace genpass {

Genpass::Genpass() {
  // register the default algorithm
  PasswordV2::registerWith(*this);
}

Genpass::~Genpass() = default;

Password&
Genpass::addPassword(std::unique_ptr<Password>&& password) {
  const auto res = passwords.insert({password->id, std::move(password)});
  if(!res.second) throw std::runtime_error("password with ID already exists");
  return *res.first->second;
}

Password&
Genpass::newPassword(const std::string& algorithm, const std::string& id) {
  const std::function<Password *()>& alg = algorithms.at(algorithm);
  std::unique_ptr<Password> password(alg());
  password->id = id;
  return addPassword(std::move(password));
}

Password&
Genpass::getPassword(const std::string& id) const {
  return *passwords.at(id);
}

void
Genpass::removePassword(const std::string& id) {
  if(!passwords.erase(id))
    throw std::out_of_range(fmt::format("no password with ID: {}", id));
}

template<>
void
Genpass::deserialize<nlohmann::json>(nlohmann::json&& in) {
  bool unknownAlg = false;

  for(const auto& pwJson : in.at("passwords")) {
    std::string algName = pwJson.at("algorithm").get<std::string>();

    auto algorithmLookup = algorithms.find(algName);
    if(algorithmLookup == algorithms.end()) {
      unknownAlg = true;
      fmt::println(stderr, "error: unknown algorithm for %s: %s",
        pwJson.at("id"), algName);
      continue;
    }
    std::function<Password *()>& algorithm = algorithmLookup->second;

    std::unique_ptr<Password> password(algorithm());
    password->deserialize(pwJson);
    addPassword(std::move(password));
  }

  if(unknownAlg) {
    fmt::println(stderr,
      "error: Some passwords could not be loaded because there is no loader."
      " This could happen if a plugin is missing.");
  }
}

nlohmann::json
Genpass::serialize() const {
  nlohmann::json ret{};
  nlohmann::json& passwordsJson = ret["passwords"] = nlohmann::json::array({});
  for(const auto& pwEntry : passwords) {
    passwordsJson += pwEntry.second->serialize();
  }
  return ret;
}

void
Genpass::clearPasswords() {
  passwords.clear();
}

void
Genpass::registerAlgorithm(const std::string& name,
  std::function<Password *()> constructor
) {
  const auto res = algorithms.insert({name, constructor});
  if(!res.second) throw std::runtime_error(fmt::format(
    "algorithm already exists: {}",
    name
  ));
}

} // namespace genpass
