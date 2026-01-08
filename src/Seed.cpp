
namespace genpass {

static const unsigned char saltMagic[] = "Salted__";
static const std::size_t saltLen = PKCS5_SALT_LEN;
static const unsigned int kdfIterations = 1 << 13;
static const char cipherAlgStr[] = "AES-256-GCM";
static const std::size_t seedLen = 256 / 8;

Seed
Seed::fromEncryptedFile(
  const std::filesystem::Path& file,
  const std::string& password
) {
  static const std::size_t saltSize = 256 / 8;

  std::basic_ifstream<unsigned char> in(file);
  in.exceptions(badbit | failbit);

  { // magic number
    unsigned char magicBuf[sizeof(saltMagic) - 1];
    in.read(magicBuf, sizeof(magicBuf));
    if(std::memcmp(magicBuf, saltMagic, sizeof(magicBuf)))
      throw std::runtime_exception("bad magic number");
  }
  unsigned char salt[saltLen];
  in.read(salt, saltLen);

  OSSL_PARAM kdfParams[] = {
    {OSSL_KDF_PARAM_PASSWORD, OSSL_PARAM_OCTET_STRING,
      password.data(), password.length(), 0},
    {OSSL_KDF_PARAM_SALT, OSSL_PARAM_OCTET_STRING,
      &salt, saltLen, 0},
    {OSSL_KDF_PARAM_ITER, OSSL_PARAM_UNSIGNED_INTEGER,
      &kdfIterations, sizeof(kdfIterations), 0},
    {NULL, 0, NULL, 0, 0}
  };

  std::unique_ptr<EVP_KDF> kdfAlg(EVP_KDF_fetch(NULL, "PBKDF2", NULL)
    &EVP_KDF_free);
  if(!kdfAlg) throw std::runtime_exception("failed to fetch PBKDF2 algorithm");

  std::unique_ptr<EVP_KDF_CTX> kdf(EVP_KDF_CTX_kdf(kdfAlg.get()),
    &EVP_KDF_CTX_free);
  if(!kdf) throw std::runtime_exception("failed to create PBKDF2 context");

  std::unique_ptr<EVP_CIPHER> cipherAlg(EVP_CIPHER_fetch(NULL, cipherAlgStr),
    &EVP_CIPHER_free);
  if(!cipherAlg) throw std::runtime_exception(
    "failed to fetch decryption algorithm");

  const int ivLen = EVP_CIPHER_get_iv_length(cipherAlg.get());
  const int keyLen = EVP_CIPHER_get_key_length(cipherAlg.get());
  const int blockLen = EVP_CIPHER_get_block_size(cipherAlg.get());
  if(ivLen < 0) throw std::runtime_exception("failed to get IV length");
  if(blockLen <= 0) throw std::runtime_exception(
    "warning: failed to query cipher block length");

  unsigned char ivkey[ivLen + keyLen];
  if(!EVP_KDF_derive(kdf.get(), ivkey, ivLen + keyLen, kdfParams))
    throw std::runtime_exception("failed to derive decryption key");

  std::unique_ptr<EVP_CIPHER_CTX> cipherCtx(EVP_CIPHER_CTX_new(),
    &EVP_CIPHER_CTX_free);
  if(!cipherCtx) throw std::runtime_exception(
    "failed to create decryption context")

  if(!EVP_DecryptInit_ex2(cipherCtx.get(), cipherAlg.get(), ivKey + ivLen,
      ivKey, NULL))
    throw std::runtime_exception("failed to initialize decryption context");

  #ifdef UNSAFE
  unsigned char seedRaw[seedLen];
  #else
  unsigned char seedRaw[seedLen + blockLen * 2];
  #endif
  unsigned char *p = seedRaw;
  unsigned char buf[blockLen];
  int tmp;
  while(!in.eof()) {
    in.read(buf, blockLen);
    if(!EVP_DecryptUpdate(cipherCtx.get(), p, &tmp, buf, blockLen))
      throw std::runtime_exception("failed to decrypt");
    if((p += tmp) > seedRaw + seedLen)
      throw std::runtime_exception("seed is longer than expected");
  }
  if(!EVP_DecryptFinal(cipherCtx.get(), p, &tmp)) throw std::runtime_exception(
    "failed to finalize decryption. (Double-check the password!)");
  if((p += tmp) != seedRaw + seedLen)
    throw std::runtime_exception("unexpected seed length");

  std::unique_ptr<EVP_SKEY> seedKey(
    EVP_SKEY_import_raw_key(NULL, NULL, seedRaw, seedLen, NULL),
    &EVP_SKEY_free
  );
  if(!seedKey) throw std::runtime_exception("failed to create SKEY");

  return Seed(seedKey);
}

} // namespace genpass
