
namespace genpass {

PasswordV2::PasswordV2(const std::string& id)
  : id(id), serial(0), length(48), postfix("aA1!"), fill('0')
{ }

std::string
PasswordV2::generate(const Seed& seed) const {

  const std::size_t genDataSize = 4 + id.length();
  unsigned char genData[genDataSize];
  unsigned char *p = genData;

  p += serialize(p, (std::int32_t)serial);
  static_assert(sizeof(*id.data()) == 1);
  std::memcpy(p, id.data(), id.length());

  std::unique_ptr<EVP_MAC> macAlg(
    EVP_MAC_fetch(NULL, "HMAC", "digest='SHA256'"),
    &EVP_MAC_free
  );
  if(!macAlg)
    throw std::runtime_exception("failed to fetch MAC algorithm");

  std::unique_ptr<EVP_MAC_CTX> mac(EVP_MAC_CTX_new(macAlg.get()),
    &EVP_MAC_CTX_free);
  if(!mac)
    throw std::runtime_exception("failed to create MAC context");

  if(!EVP_MAC_init_SKEY(mac.get(), seed.key, NULL))
    throw std::runtime_exception("failure in MAC initialization");

  if(!EVP_MAC_update(mac.get(), genData))
    throw std::runtime_exception("failure in MAC update");

  std::size_t macSize = EVP_MAC_CTX_get_mac_size(mac.get());
  if(!macSize)
    throw std::runtime_exception("failed to query MAC size");

  unsigned char macOut[macSize];
  std::size_t macOutLen;
  if(!EVP_MAC_final(mac.get(), macOut, &macOutLen, macSize))
    throw std::runtime_exception("failed to finalize MAC");

  unsigned char encoded[(macOutLen / 3 + 1) * 4];
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

} // namespace genpass
