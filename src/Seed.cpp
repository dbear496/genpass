/* ---------------------------------------------------------------------- *\
 * src/Seed.cpp
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

#include "Seed.hpp"

#include <cstddef>
#include <openssl/kdf.h>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <cassert>
#include <functional>
#include <openssl/core_names.h>
#include <openssl/evp.h>

#include "util/ossl_ptr.hpp"

namespace genpass {

static const unsigned char saltMagic[] = "Salted__";
static const std::size_t saltLen = PKCS5_SALT_LEN;
static const unsigned int kdfIterations = 1 << 13;
static const char cipherAlgStr[] = "AES-256-ECB";
static const std::size_t seedLen = 256 / 8;

Seed
Seed::fromEncryptedFile(
  const std::filesystem::path& file,
  const std::string& password
) {
  // setup input stream
  std::basic_ifstream<unsigned char> in(file);
  in.exceptions(std::ios_base::badbit | std::ios_base::failbit);

  { // read and verify magic number
    unsigned char magicBuf[sizeof(saltMagic) - 1];
    in.read(magicBuf, sizeof(magicBuf));
    if(std::memcmp(magicBuf, saltMagic, sizeof(magicBuf)))
      throw std::runtime_error("bad magic number");
  }

  // read salt
  unsigned char salt[saltLen];
  in.read(salt, saltLen);

  // fetch KDF
  ossl_unique_ptr<EVP_KDF> kdfAlg(
    EVP_KDF_fetch(NULL, "PBKDF2", NULL),
    &EVP_KDF_free);
  if(!kdfAlg) throw std::runtime_error("failed to fetch PBKDF2 algorithm");

  // create KDF
  ossl_unique_ptr<EVP_KDF_CTX> kdf(EVP_KDF_CTX_new(kdfAlg.get()),
    &EVP_KDF_CTX_free);
  if(!kdf) throw std::runtime_error("failed to create PBKDF2 context");

  // fetch cipher
  ossl_unique_ptr<EVP_CIPHER> cipherAlg(
    EVP_CIPHER_fetch(NULL, cipherAlgStr, NULL),
    &EVP_CIPHER_free);
  if(!cipherAlg) throw std::runtime_error(
    "failed to fetch decryption algorithm");

  // query cipher parameters
  const int ivLen = EVP_CIPHER_get_iv_length(cipherAlg.get());
  const int keyLen = EVP_CIPHER_get_key_length(cipherAlg.get());
  assert(EVP_CIPHER_get_block_size(cipherAlg.get()) == 16);
  if(ivLen < 0) throw std::runtime_error("failed to get IV length");

  // derive key
  OSSL_PARAM kdfParams[] = {
    {OSSL_KDF_PARAM_PASSWORD, OSSL_PARAM_OCTET_STRING,
      const_cast<std::string&>(password).data(), password.length(), 0},
    {OSSL_KDF_PARAM_SALT, OSSL_PARAM_OCTET_STRING,
      &salt, saltLen, 0},
    {OSSL_KDF_PARAM_ITER, OSSL_PARAM_UNSIGNED_INTEGER,
      &const_cast<unsigned int&>(kdfIterations), sizeof(kdfIterations), 0},
    {NULL, 0, NULL, 0, 0}
  };
  unsigned char ivkey[ivLen + keyLen];
  if(!EVP_KDF_derive(kdf.get(), ivkey, ivLen + keyLen, kdfParams))
    throw std::runtime_error("failed to derive decryption key");

  // create cipher
  ossl_unique_ptr<EVP_CIPHER_CTX> cipherCtx(EVP_CIPHER_CTX_new(),
    &EVP_CIPHER_CTX_free);
  if(!cipherCtx) throw std::runtime_error(
    "failed to create decryption context");
  EVP_CIPHER_CTX_set_padding(cipherCtx.get(), 1);

  // initialize cipher
  if(!EVP_DecryptInit_ex2(cipherCtx.get(), cipherAlg.get(), ivkey + ivLen,
      ivkey, NULL))
    throw std::runtime_error("failed to initialize decryption context");

  // read encrypted seed
  unsigned char seedRaw[seedLen];
  in.read(seedRaw, seedLen);

  // decrypt seed
  int tmp;
  if(!EVP_DecryptUpdate(cipherCtx.get(), seedRaw, &tmp, seedRaw, seedLen))
    throw std::runtime_error("failed to decrypt");
  assert(tmp == seedLen);
  if(!EVP_DecryptFinal(cipherCtx.get(), NULL, &tmp))
    throw std::runtime_error(
      "failed to finalize decryption. (Make sure the password is correct!)");
  assert(tmp == 0);

  ossl_unique_ptr<EVP_SKEY> seedKey(
    EVP_SKEY_import_raw_key(NULL, NULL, seedRaw, seedLen, NULL),
    &EVP_SKEY_free
  );
  if(!seedKey) throw std::runtime_error("failed to create SKEY");

  return Seed(std::move(seedKey));
}

} // namespace genpass
