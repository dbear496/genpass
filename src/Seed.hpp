

namespace genpass {
class Seed {
public:
  Seed(std::unique_ptr<EVP_SKEY>&& key)
    : key(std::move(key))
  { }
  ~Seed() { }

  const EVP_SKEY *getKey() const { return key.get(); }

  static Seed fromEncryptedFile(
    std::filesystem::Path file,
    std::string password
  );

private:

  const std::unique_ptr<EVP_SKEY> key;
}
