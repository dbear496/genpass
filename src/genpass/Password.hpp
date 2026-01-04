


namespace genpass {

class Password {
public:
  Password(const std::string& id);
  virtual ~Password();

  virtual std::string generate(const Seed& seed) const = 0;

  virtual const std::string& algorithmName() const = 0;
  virtual nlohmann::json serialize() const;

  std::string id;
  int serial;
};

class PasswordV2 : Password {
public:

  PasswordV2(const std::string& id);
  virtual ~PasswordV2();

  virtual std::string generate(const Seed& seed) const;
  virtual std::string prepare(const std::string& base) const;

  virtual const std::string& algorithmName() const { return algName; }
  virtual nlohmann::json serialize() const;

  std::size_t length;
  std::string postfix;
  std::unordered_set<char> bannedChars;
  char fill;

private:
  static const std::string algName = "genpass-2.0";
};
} // namespace genpass
