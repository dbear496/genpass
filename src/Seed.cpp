/* ---------------------------------------------------------------------- *\
 * src/Seed.cpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2025-2026 David Bears <dbear4q@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
\* ---------------------------------------------------------------------- */

#include "genpass/Seed.hpp"

#include <openssl/core.h>        // for OSSL_PARAM_OCTET_STRING, OSSL_PARAM_...
#include <openssl/core_names.h>  // for OSSL_KDF_PARAM_ITER, OSSL_KDF_PARAM_...
#include <openssl/evp.h>         // for EVP_CIPHER_CTX_new, EVP_CIPHER_CTX_s...
#include <openssl/kdf.h>         // for EVP_KDF_CTX_new, EVP_KDF_derive, EVP...
#include <openssl/rand.h>
#include <openssl/types.h>       // for EVP_CIPHER, EVP_CIPHER_CTX, EVP_KDF
#include <cassert>               // for assert
#include <cstring>               // for NULL, memcmp, size_t
#include <fstream>               // for basic_ifstream, basic_istream::read
#include <stdexcept>             // for runtime_error
#include <fmt/format.h>

#include "genpass/detail/ossl_ptr.hpp"     // for ossl_unique_ptr

namespace genpass {

static const std::size_t seedLen = 256 / 8;

static const unsigned char enc_saltMagic[8] = {'S','a','l','t','e','d','_','_'};
static const char enc_kdfName[] = "PBKDF2";
static const std::size_t enc_saltLen = PKCS5_SALT_LEN;
static const unsigned int enc_kdfIter = 1 << 16;
static const char enc_cipherName[] = "AES-256-ECB";

static const char gen_kdfName[] = "PBKDF2";
static const unsigned char gen_kdfSalt[] =
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const unsigned int gen_kdfIter = 1 << 24;

static inline ossl_unique_ptr<EVP_CIPHER_CTX>
initFileEncryptionCipher(bool encrypt, const std::string& password,
  const unsigned char *salt, std::size_t saltLen
) {
  // fetch KDF
  ossl_unique_ptr<EVP_KDF> kdfAlg(
    EVP_KDF_fetch(NULL, enc_kdfName, NULL),
    &EVP_KDF_free);
  if(!kdfAlg) throw std::runtime_error("failed to fetch PBKDF2 algorithm");

  // create KDF
  ossl_unique_ptr<EVP_KDF_CTX> kdf(EVP_KDF_CTX_new(kdfAlg.get()),
    &EVP_KDF_CTX_free);
  if(!kdf) throw std::runtime_error("failed to create PBKDF2 context");

  // fetch cipher
  ossl_unique_ptr<EVP_CIPHER> cipherAlg(
    EVP_CIPHER_fetch(NULL, enc_cipherName, NULL),
    &EVP_CIPHER_free);
  if(!cipherAlg) throw std::runtime_error("failed to fetch cipher algorithm");

  // query cipher parameters
  const int ivLen = EVP_CIPHER_get_iv_length(cipherAlg.get());
  const int keyLen = EVP_CIPHER_get_key_length(cipherAlg.get());
  assert(EVP_CIPHER_get_block_size(cipherAlg.get()) == 16);
  if(ivLen < 0) throw std::runtime_error("failed to get IV length");

  // derive key
  OSSL_PARAM kdfParams[] = {
    {OSSL_KDF_PARAM_PASSWORD, OSSL_PARAM_OCTET_STRING,
      const_cast<char *>(password.data()), password.length(), 0},
    {OSSL_KDF_PARAM_SALT, OSSL_PARAM_OCTET_STRING,
      const_cast<unsigned char *>(salt), saltLen, 0},
    {OSSL_KDF_PARAM_ITER, OSSL_PARAM_UNSIGNED_INTEGER,
      &const_cast<unsigned int&>(enc_kdfIter), sizeof(enc_kdfIter), 0},
    {NULL, 0, NULL, 0, 0}
  };
  unsigned char ivkey[ivLen + keyLen];
  if(!EVP_KDF_derive(kdf.get(), ivkey, ivLen + keyLen, kdfParams))
    throw std::runtime_error("failed to derive cipher key");

  // create cipher
  ossl_unique_ptr<EVP_CIPHER_CTX> cipherCtx(EVP_CIPHER_CTX_new(),
    &EVP_CIPHER_CTX_free);
  if(!cipherCtx) throw std::runtime_error("failed to create cipher context");

  // use a block of all padding to verify that the password is correct
  EVP_CIPHER_CTX_set_padding(cipherCtx.get(), 1);

  // initialize cipher
  if(!EVP_CipherInit_ex2(cipherCtx.get(), cipherAlg.get(), ivkey + ivLen,
      ivkey, +encrypt, NULL)
  ) throw std::runtime_error("failed to initialize cipher context");

  return cipherCtx;
}

void
Seed::toEncryptedFile(
  const std::filesystem::path& file,
  const std::string& password
) {
  /*
  equivalent openssl command:
  $ openssl enc -AES-256-ECB -e -pbkdf2 -iter 65536 -pass pass:<password>
  */

  // generate a random salt
  unsigned char salt[enc_saltLen];
  if(RAND_bytes(salt, enc_saltLen) <= 0) throw std::runtime_error(
    "failed to generate random salt");

  // get the raw key bytes
  const unsigned char *seedRaw;
  std::size_t seedLenTmp;
  if(!EVP_SKEY_get0_raw_key(key.get(), &seedRaw, &seedLenTmp))
    throw std::runtime_error("failed to export raw seed");
  if(seedLenTmp != seedLen)
    throw std::runtime_error(fmt::format(
      "unexpected seed length {}, expected {}", seedLenTmp, seedLen));

  // create the cipher
  ossl_unique_ptr<EVP_CIPHER_CTX> cipherCtx =
    initFileEncryptionCipher(true, password, salt, enc_saltLen);

  // encrypt the seed
  unsigned char seedEnc[seedLen + 16]; // with padding
  int outl;
  if(!EVP_EncryptUpdate(cipherCtx.get(), seedEnc, &outl, seedRaw, seedLen))
    throw std::runtime_error("failure during seed encryption");
  assert(outl == seedLen);
  if(!EVP_EncryptFinal(cipherCtx.get(), seedEnc, &outl))
    throw std::runtime_error("failed to finalize encryption");
  assert(outl == 16);

  // write everything to the file
  std::basic_ofstream<unsigned char> out(file,
    std::ios::out | std::ios::binary | std::ios::trunc);
  out.exceptions(std::ios::badbit | std::ios::failbit);
  out.write(enc_saltMagic, sizeof(enc_saltMagic));
  out.write(salt, enc_saltLen);
  out.write(seedEnc, seedLen + 16);
}

Seed
Seed::fromEncryptedFile(
  const std::filesystem::path& file,
  const std::string& password
) {
  /*
  equivalent openssl command:
  $ openssl enc -AES-256-ECB -d -pbkdf2 -iter 65536 -pass pass:<password>
  */

  // setup input stream
  std::basic_ifstream<unsigned char> in(file);
  in.exceptions(std::ios::badbit | std::ios::failbit);

  { // read and verify magic number
    unsigned char magicBuf[sizeof(enc_saltMagic)];
    in.read(magicBuf, sizeof(magicBuf));
    if(std::memcmp(magicBuf, enc_saltMagic, sizeof(magicBuf)))
      throw std::runtime_error("bad magic number");
  }

  // read salt
  unsigned char salt[enc_saltLen];
  in.read(salt, enc_saltLen);

  // create cipher
  ossl_unique_ptr<EVP_CIPHER_CTX> cipherCtx =
    initFileEncryptionCipher(false, password, salt, enc_saltLen);

  // read encrypted seed
  unsigned char seedRaw[seedLen + 16]; // with padding
  in.read(seedRaw, seedLen + 16);

  // decrypt seed
  int tmp;
  if(!EVP_DecryptUpdate(cipherCtx.get(), seedRaw, &tmp, seedRaw, seedLen + 16))
    throw std::runtime_error("failure during seed decryption");
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

Seed
Seed::fromPassword(const std::string& password) {
  /*
  equivalent openssl command:
  $ openssl kdf -keylen 32 -kdfopt hexsalt:0000000000000000 \
    -kdfopt iter:16777216 -kdfopt pass:<password> -binary PBKDF2
  */

  // fetch KDF
  ossl_unique_ptr<EVP_KDF> kdfAlg(
    EVP_KDF_fetch(NULL, gen_kdfName, NULL),
    &EVP_KDF_free);
  if(!kdfAlg) throw std::runtime_error("failed to fetch PBKDF2 algorithm");

  // create KDF
  ossl_unique_ptr<EVP_KDF_CTX> kdf(EVP_KDF_CTX_new(kdfAlg.get()),
    &EVP_KDF_CTX_free);
  if(!kdf) throw std::runtime_error("failed to create PBKDF2 context");

  // derive key
  OSSL_PARAM kdfParams[] = {
    {OSSL_KDF_PARAM_PASSWORD, OSSL_PARAM_OCTET_STRING,
      const_cast<char *>(password.data()), password.length(), 0},
    {OSSL_KDF_PARAM_SALT, OSSL_PARAM_OCTET_STRING,
      const_cast<unsigned char *>(gen_kdfSalt), sizeof(gen_kdfSalt), 0},
    {OSSL_KDF_PARAM_ITER, OSSL_PARAM_UNSIGNED_INTEGER,
      const_cast<unsigned int *>(&gen_kdfIter), sizeof(gen_kdfIter), 0},
    {NULL, 0, NULL, 0, 0}
  };
  unsigned char seedRaw[seedLen];
  if(!EVP_KDF_derive(kdf.get(), seedRaw, seedLen, kdfParams))
    throw std::runtime_error("failed to derive seed");

  ossl_unique_ptr<EVP_SKEY> seedKey(
    EVP_SKEY_import_raw_key(NULL, NULL, seedRaw, seedLen, NULL),
    &EVP_SKEY_free
  );
  if(!seedKey) throw std::runtime_error("failed to create SKEY");

  return Seed(std::move(seedKey));
}

} // namespace genpass
