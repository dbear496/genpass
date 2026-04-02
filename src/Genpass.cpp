/* ------------------------------------------------------------------------ *\
 * src/Genpass.cpp
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

#include "genpass/Genpass.hpp"

#include <fmt/base.h>            // for println
#include <fmt/format.h>          // for format, native_formatter::format
#include <cassert>               // for assert
#include <cstdio>                // for stderr
#include <set>                   // for set
#include <stdexcept>             // for logic_error, out_of_range, runtime_e...
#include <utility>               // for pair, move

#include "genpass/Password.hpp"  // for Password, PasswordV2
#include "genpass/detail/fmt_nlohmann.hpp"

namespace genpass {

Genpass::Genpass() {
  // register the default algorithm
  PasswordV2::registerWith(*this);
}

Genpass::~Genpass() = default;

Password&
Genpass::addPassword(std::unique_ptr<Password>&& password) {
  const auto res = passwords.insert({password->id, std::move(password)});
  if(!res.second) throw std::logic_error("password with ID already exists");
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

std::size_t
Genpass::passwordCount() const {
  return passwords.size();
}

Password *
Genpass::getPasswordPtr(const std::string& id) const {
  auto find = passwords.find(id);
  if(find == passwords.end()) return nullptr;
  else return find->second.get();
}

void
Genpass::removePassword(const std::string& id) {
  if(!passwords.erase(id))
    throw std::out_of_range(fmt::format("no password with ID: {}", id));
}

void
Genpass::updateId(const std::string& oldId) {
  auto pwIt = passwords.find(oldId);
  if(pwIt == passwords.end())
    throw std::out_of_range(fmt::format(
      "no password registered with ID: {}", oldId));
  const std::string& newId = pwIt->second->id;
  if(newId == oldId) return;
  if(passwords.find(newId) != passwords.end())
    throw std::logic_error(fmt::format(
      "a password with this ID already exists: {}", newId));
  auto node = passwords.extract(pwIt);
  node.key() = newId;
  auto result = passwords.insert(std::move(node));
  if(!result.inserted)
    throw std::runtime_error("failed while updating password ID");
}

void
Genpass::updateAllIds() {
  using pws = decltype(passwords);

  for(pws::iterator it = passwords.begin(); it != passwords.end(); ++it) {
    const std::string& id = it->second->id;
    if(it->first == id) {
      ++it;
      continue;
    }

    std::set<std::string> dsts;
    pws::iterator jt = it;
    // first verify that reordering is possible without conflicts
    while((jt = passwords.find(jt->second->id)) != passwords.end() && jt != it)
      if(!dsts.insert(jt->second->id).second)
        throw std::logic_error(fmt::format(
          "found duplicate password IDs: {}", jt->second->id));
    // ...then do the actual reordering
    while((jt = passwords.find(it->second->id)) != passwords.end() && jt != it)
      std::swap(it->second, jt->second);
    // the last element may need to be moved instead of swapped
    if(jt == passwords.end()) {
      pws::node_type node = passwords.extract(it++);
      node.key() = id;
      auto result = passwords.insert(std::move(node));
      assert(result.inserted);
    }
    else {
      ++it;
    }
  }
}

Genpass::PasswordIterator
Genpass::passwords_begin() {
  return passwords.begin();
}
Genpass::PasswordIterator
Genpass::passwords_end() {
  return passwords.end();
}
Genpass::ConstPasswordIterator
Genpass::passwords_cbegin() const {
  return passwords.cbegin();
}
Genpass::ConstPasswordIterator
Genpass::passwords_cend() const {
  return passwords.cend();
}

Genpass::AlgorithmNameIterator
Genpass::algorithms_begin() const {
  return algorithms.cbegin();
}
Genpass::AlgorithmNameIterator
Genpass::algorithms_end() const {
  return algorithms.cend();
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
      fmt::println(stderr, "error: unknown algorithm for {:s}: {:s}",
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
