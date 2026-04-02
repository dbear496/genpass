/* ------------------------------------------------------------------------ *\
 * src/Seed.cpp
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

#include "genpass/Seed.hpp"

#include <openssl/core.h>               // for OSSL_PARAM_OCTET_STRING, OSSL...
#include <openssl/core_names.h>         // for OSSL_KDF_PARAM_ITER, OSSL_KDF...
#include <openssl/evp.h>                // for EVP_CIPHER_CTX_free, EVP_CIPH...
#include <openssl/kdf.h>                // for EVP_KDF_CTX_free, EVP_KDF_CTX...
#include <openssl/rand.h>               // for RAND_bytes
#include <openssl/types.h>              // for EVP_CIPHER_CTX, EVP_KDF, EVP_...
#include <cassert>                      // for assert
#include <cstring>                      // for memcmp
#include <fstream>                      // for basic_ifstream, basic_ofstream
#include <stdexcept>                    // for runtime_error

#include "genpass/detail/ossl_ptr.hpp"  // for ossl_unique_ptr
#include "genpass/exceptions.hpp"       // for WrongKeyException

namespace genpass {

static const unsigned char enc_saltMagic[8] = {'S','a','l','t','e','d','_','_'};
static const char enc_kdfName[] = "PBKDF2";
static const char enc_kdfDigest[] = "SHA256";
static const std::size_t enc_saltLen = PKCS5_SALT_LEN;
static const unsigned int enc_kdfIter = 1 << 16;
static const char enc_cipherName[] = "AES-256-ECB";

static const char gen_kdfName[] = "PBKDF2";
static const char gen_kdfDigest[] = "SHA256";
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
  if(!kdfAlg) throw ossl_error();

  // create KDF
  ossl_unique_ptr<EVP_KDF_CTX> kdf(EVP_KDF_CTX_new(kdfAlg.get()),
    &EVP_KDF_CTX_free);
  if(!kdf) throw ossl_error();

  // fetch cipher
  ossl_unique_ptr<EVP_CIPHER> cipherAlg(
    EVP_CIPHER_fetch(NULL, enc_cipherName, NULL),
    &EVP_CIPHER_free);
  if(!cipherAlg) throw ossl_error();

  // query cipher parameters
  const int ivLen = EVP_CIPHER_get_iv_length(cipherAlg.get());
  const int keyLen = EVP_CIPHER_get_key_length(cipherAlg.get());
  assert(EVP_CIPHER_get_block_size(cipherAlg.get()) == 16);
  if(ivLen < 0) throw ossl_error();

  // derive key
  OSSL_PARAM kdfParams[] = {
    {OSSL_KDF_PARAM_PASSWORD, OSSL_PARAM_OCTET_STRING,
      const_cast<char *>(password.data()), password.length(), 0},
    {OSSL_KDF_PARAM_SALT, OSSL_PARAM_OCTET_STRING,
      const_cast<unsigned char *>(salt), saltLen, 0},
    {OSSL_KDF_PARAM_DIGEST, OSSL_PARAM_UTF8_STRING,
      const_cast<char *>(enc_kdfDigest), sizeof(enc_kdfDigest)-1, 0},
    {OSSL_KDF_PARAM_ITER, OSSL_PARAM_UNSIGNED_INTEGER,
      &const_cast<unsigned int&>(enc_kdfIter), sizeof(enc_kdfIter), 0},
    {NULL, 0, NULL, 0, 0}
  };
  unsigned char ivkey[ivLen + keyLen];
  if(!EVP_KDF_derive(kdf.get(), ivkey, ivLen + keyLen, kdfParams))
    throw ossl_error();

  // create cipher
  ossl_unique_ptr<EVP_CIPHER_CTX> cipherCtx(EVP_CIPHER_CTX_new(),
    &EVP_CIPHER_CTX_free);
  if(!cipherCtx) throw ossl_error();

  // use a block of all padding to verify that the password is correct
  EVP_CIPHER_CTX_set_padding(cipherCtx.get(), 1);

  // initialize cipher
  if(!EVP_CipherInit_ex2(cipherCtx.get(), cipherAlg.get(), ivkey + ivLen,
      ivkey, +encrypt, NULL)
  ) throw ossl_error();

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
  if(RAND_bytes(salt, enc_saltLen) <= 0) throw ossl_error();

  // create the cipher
  ossl_unique_ptr<EVP_CIPHER_CTX> cipherCtx =
    initFileEncryptionCipher(true, password, salt, enc_saltLen);

  // encrypt the seed
  unsigned char seedEnc[Seed::SIZE + 16]; // with padding
  int outl;
  if(!EVP_EncryptUpdate(cipherCtx.get(), seedEnc, &outl, getData(), Seed::SIZE))
    throw ossl_error();
  assert(outl == Seed::SIZE);
  if(!EVP_EncryptFinal(cipherCtx.get(), seedEnc, &outl)) throw ossl_error();
  assert(outl == 16);

  // write everything to the file
  std::ofstream out(file,
    std::ios::out | std::ios::binary | std::ios::trunc);
  out.exceptions(std::ios::badbit | std::ios::failbit);
  out.write((char *)enc_saltMagic, sizeof(enc_saltMagic));
  out.write((char *)salt, enc_saltLen);
  out.write((char *)seedEnc, Seed::SIZE + 16);
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
  std::ifstream in(file);
  in.exceptions(std::ios::badbit | std::ios::failbit);

  { // read and verify magic number
    unsigned char magicBuf[sizeof(enc_saltMagic)];
    in.read((char *)magicBuf, sizeof(magicBuf));
    if(std::memcmp(magicBuf, enc_saltMagic, sizeof(magicBuf)))
      throw std::runtime_error("bad magic number");
  }

  // read salt
  unsigned char salt[enc_saltLen];
  in.read((char *)salt, enc_saltLen);

  // create cipher
  ossl_unique_ptr<EVP_CIPHER_CTX> cipherCtx =
    initFileEncryptionCipher(false, password, salt, enc_saltLen);

  // read encrypted seed
  unsigned char raw[Seed::SIZE + 16]; // with padding
  in.read((char *)raw, Seed::SIZE + 16);

  // decrypt seed
  auto data = Seed::allocData();
  int len;
  if(!EVP_DecryptUpdate(cipherCtx.get(), data.get(), &len, raw, Seed::SIZE+16))
    throw ossl_error();
  assert(len == Seed::SIZE);
  if(!EVP_DecryptFinal(cipherCtx.get(), NULL, &len))
    throw WrongKeyException();
  assert(len == 0);

  return Seed(std::move(data));
}

Seed
Seed::fromPassword(const std::string& password) {
  /*
  equivalent openssl command:
  $ openssl kdf -keylen 32 -digest SHA256 -kdfopt hexsalt:0000000000000000 \
    -kdfopt iter:16777216 -kdfopt pass:<password> -binary PBKDF2
  */

  // fetch KDF
  ossl_unique_ptr<EVP_KDF> kdfAlg(
    EVP_KDF_fetch(NULL, gen_kdfName, NULL),
    &EVP_KDF_free);
  if(!kdfAlg) throw ossl_error();

  // create KDF
  ossl_unique_ptr<EVP_KDF_CTX> kdf(EVP_KDF_CTX_new(kdfAlg.get()),
    &EVP_KDF_CTX_free);
  if(!kdf) throw ossl_error();

  // derive key
  OSSL_PARAM kdfParams[] = {
    {OSSL_KDF_PARAM_PASSWORD, OSSL_PARAM_OCTET_STRING,
      const_cast<char *>(password.data()), password.length(), 0},
    {OSSL_KDF_PARAM_SALT, OSSL_PARAM_OCTET_STRING,
      const_cast<unsigned char *>(gen_kdfSalt), sizeof(gen_kdfSalt), 0},
    {OSSL_KDF_PARAM_DIGEST, OSSL_PARAM_UTF8_STRING,
      const_cast<char *>(gen_kdfDigest), sizeof(gen_kdfDigest)-1, 0},
    {OSSL_KDF_PARAM_ITER, OSSL_PARAM_UNSIGNED_INTEGER,
      const_cast<unsigned int *>(&gen_kdfIter), sizeof(gen_kdfIter), 0},
    {NULL, 0, NULL, 0, 0}
  };
  auto data = Seed::allocData();
  if(!EVP_KDF_derive(kdf.get(), data.get(), Seed::SIZE, kdfParams))
    throw ossl_error();

  return Seed(std::move(data));
}

} // namespace genpass
