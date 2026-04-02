/* ------------------------------------------------------------------------ *\
 * src/Password.cpp
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

#include "genpass/Password.hpp"

#include <nlohmann/detail/json_ref.hpp>  // for json_ref
#include <nlohmann/json.hpp>             // for basic_json
#include <openssl/core.h>                // for OSSL_PARAM_UTF8_STRING, ossl...
#include <openssl/core_names.h>          // for OSSL_MAC_PARAM_DIGEST
#include <openssl/evp.h>                 // for EVP_EncodeBlock, EVP_MAC_CTX...
#include <openssl/types.h>               // for EVP_MAC, EVP_MAC_CTX, OSSL_P...
#include <cstring>                       // for memcpy
#include <functional>                    // for function
#include <stdexcept>                     // for runtime_error, invalid_argument

#include "genpass/Genpass.hpp"           // for Genpass
#include "genpass/Seed.hpp"              // for Seed
#include "genpass/detail/ossl_ptr.hpp"   // for ossl_unique_ptr
#include "genpass/detail/serialize.hpp"  // for serialize
#include "genpass/exceptions.hpp"

namespace genpass {

Password::Password()
  : Password("")
{ }

Password::Password(const std::string& id)
  : id(id), serial(0)
{ }

Password::~Password() = default;

nlohmann::json
Password::serialize() const {
  return nlohmann::json{
    {"algorithm", algorithmName()},
    {"id", id},
    {"serial", serial},
    {"note", note}
  };
}

void
Password::deserialize(const nlohmann::json& json) {
  const std::string alg = json.at("algorithm").get<std::string>();
  if(alg != algorithmName()) throw std::runtime_error("algorithm mismatch");

  json.at("id").get_to(id);
  json.at("serial").get_to(serial);
  json.at("note").get_to(note);
}

void
to_json(nlohmann::json& json, const Password& password) {
  json = password.serialize();
}

void
from_json(const nlohmann::json& json, Password& password) {
  password.deserialize(json);
}

PasswordV2::PasswordV2() : PasswordV2("") { }

PasswordV2::PasswordV2(const std::string& id)
  : Password(id), length(48), postfix("aA1!"), fill('0')
{ }

PasswordV2::~PasswordV2() = default;

void
PasswordV2::registerWith(Genpass& genpass) {
  genpass.registerAlgorithm(algName, [](){ return new PasswordV2(); });
}

std::string
PasswordV2::generate(const Seed& seed) const {

  static const char macDgst[] = "SHA256";

  const std::size_t genDataSize = sizeof(std::int32_t) + id.length();
  unsigned char genData[genDataSize];
  unsigned char *p = genData;

  p += genpass::serialize(p, (std::int32_t)serial);
  static_assert(sizeof(*id.data()) == 1);
  std::memcpy(p, id.data(), id.length());

  ossl_unique_ptr<EVP_MAC> macAlg(
    EVP_MAC_fetch(NULL, "HMAC", NULL),
    &EVP_MAC_free
  );
  if(!macAlg)
    throw std::runtime_error("failed to fetch MAC algorithm");

  ossl_unique_ptr<EVP_MAC_CTX> mac(EVP_MAC_CTX_new(macAlg.get()),
    &EVP_MAC_CTX_free);
  if(!mac) throw ossl_error();

  OSSL_PARAM macParams[] = {
    {OSSL_MAC_PARAM_DIGEST, OSSL_PARAM_UTF8_STRING,
      const_cast<char *>(macDgst), sizeof(macDgst)-1, 0},
    {NULL, 0, NULL, 0, 0}
  };
  if(!EVP_MAC_init(mac.get(), seed.getData(), Seed::SIZE, macParams))
    throw ossl_error();

  if(!EVP_MAC_update(mac.get(), genData, genDataSize)) throw ossl_error();

  std::size_t macSize = EVP_MAC_CTX_get_mac_size(mac.get());
  if(!macSize) throw ossl_error();

  unsigned char macOut[macSize];
  std::size_t macOutLen;
  if(!EVP_MAC_final(mac.get(), macOut, &macOutLen, macSize)) throw ossl_error();

  unsigned char encoded[(macOutLen + 2) / 3 * 4 + 1];
  EVP_EncodeBlock(encoded, macOut, macOutLen);

  return prepare((char *)encoded);
}

std::string
PasswordV2::prepare(const std::string& base) const {
  std::string pw(base);

  // remove banned chars
  for(std::size_t i = 0; i < pw.length(); i++) {
    if(bannedChars.find(pw[i]) != bannedChars.end()) {
      pw.erase(i--, 1);
    }
  }

  // truncate or pad to the preferred length
  if(length < postfix.length())
    throw std::invalid_argument("postfix too long");
  pw.resize(length - postfix.length(), fill);

  // append postfix
  pw += postfix;

  return pw;
}

void
PasswordV2::validate() const {
  if(postfix.length() > length) throw std::invalid_argument(
    "length should not be shorter than the static prefix");
}

nlohmann::json
PasswordV2::serialize() const {
  nlohmann::json json = Password::serialize();
  json.update(nlohmann::json{
    {"length", length},
    {"postfix", postfix},
    {"bannedChars", bannedChars},
    {"fill", fill}
  });
  return json;
}

void
PasswordV2::deserialize(const nlohmann::json& json) {
  Password::deserialize(json);

  json.at("length").get_to(length);
  json.at("postfix").get_to(postfix);
  json.at("bannedChars").get_to(bannedChars);
  json.at("fill").get_to(fill);
}

} // namespace genpass
