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


namespace genpass {

static const unsigned char saltMagic[] = "Salted__";
static const std::size_t saltLen = PKCS5_SALT_LEN;
static const unsigned int kdfIterations = 1 << 13;
static const char cipherAlgStr[] = "AES-256-ECB";
static const std::size_t seedLen = 256 / 8;

Seed
Seed::fromEncryptedFile(
  const std::filesystem::Path& file,
  const std::string& password
) {
  // setup input stream
  std::basic_ifstream<unsigned char> in(file);
  in.exceptions(badbit | failbit);

  { // read and verify magic number
    unsigned char magicBuf[sizeof(saltMagic) - 1];
    in.read(magicBuf, sizeof(magicBuf));
    if(std::memcmp(magicBuf, saltMagic, sizeof(magicBuf)))
      throw std::runtime_exception("bad magic number");
  }

  // read salt
  unsigned char salt[saltLen];
  in.read(salt, saltLen);

  // fetch KDF
  std::unique_ptr<EVP_KDF> kdfAlg(EVP_KDF_fetch(NULL, "PBKDF2", NULL)
    &EVP_KDF_free);
  if(!kdfAlg) throw std::runtime_exception("failed to fetch PBKDF2 algorithm");

  // create KDF
  std::unique_ptr<EVP_KDF_CTX> kdf(EVP_KDF_CTX_kdf(kdfAlg.get()),
    &EVP_KDF_CTX_free);
  if(!kdf) throw std::runtime_exception("failed to create PBKDF2 context");

  // fetch cipher
  std::unique_ptr<EVP_CIPHER> cipherAlg(EVP_CIPHER_fetch(NULL, cipherAlgStr),
    &EVP_CIPHER_free);
  if(!cipherAlg) throw std::runtime_exception(
    "failed to fetch decryption algorithm");

  // query cipher parameters
  const int ivLen = EVP_CIPHER_get_iv_length(cipherAlg.get());
  const int keyLen = EVP_CIPHER_get_key_length(cipherAlg.get());
  assert(EVP_CIPHER_get_block_size(cipherAlg.get()) == 16);
  if(ivLen < 0) throw std::runtime_exception("failed to get IV length");

  // derive key
  OSSL_PARAM kdfParams[] = {
    {OSSL_KDF_PARAM_PASSWORD, OSSL_PARAM_OCTET_STRING,
      password.data(), password.length(), 0},
    {OSSL_KDF_PARAM_SALT, OSSL_PARAM_OCTET_STRING,
      &salt, saltLen, 0},
    {OSSL_KDF_PARAM_ITER, OSSL_PARAM_UNSIGNED_INTEGER,
      &kdfIterations, sizeof(kdfIterations), 0},
    {NULL, 0, NULL, 0, 0}
  };
  unsigned char ivkey[ivLen + keyLen];
  if(!EVP_KDF_derive(kdf.get(), ivkey, ivLen + keyLen, kdfParams))
    throw std::runtime_exception("failed to derive decryption key");

  // create cipher
  std::unique_ptr<EVP_CIPHER_CTX> cipherCtx(EVP_CIPHER_CTX_new(),
    &EVP_CIPHER_CTX_free);
  if(!cipherCtx) throw std::runtime_exception(
    "failed to create decryption context")
  EVP_CIPHER_CTX_set_padding(cipherCtx.get(), 1);

  // initialize cipher
  if(!EVP_DecryptInit_ex2(cipherCtx.get(), cipherAlg.get(), ivKey + ivLen,
      ivKey, NULL))
    throw std::runtime_exception("failed to initialize decryption context");

  // read encrypted seed
  unsigned char seedRaw[seedLen];
  in.read(seedRaw, seedLen);

  // decrypt seed
  int tmp;
  if(!EVP_DecryptUpdate(cipherCtx.get(), seedRaw, &tmp, seedRaw, seedLen))
    throw std::runtime_exception("failed to decrypt");
  assert(tmp == seedLen);
  if(!EVP_DecryptFinal(cipherCtx.get(), NULL, &tmp))
    throw std::runtime_exception(
      "failed to finalize decryption. (Make sure the password is correct!)");
  assert(tmp == 0);

  std::unique_ptr<EVP_SKEY> seedKey(
    EVP_SKEY_import_raw_key(NULL, NULL, seedRaw, seedLen, NULL),
    &EVP_SKEY_free
  );
  if(!seedKey) throw std::runtime_exception("failed to create SKEY");

  return Seed(seedKey);
}

} // namespace genpass
