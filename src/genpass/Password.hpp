


namespace genpass {

class Password {
public:
  Password();
  virtual ~Password();

  virtual std::string generate(const Seed& seed) const = 0;

  std::string id;
  int serial;
};

class PasswordV2 : Password {
public:

  PasswordV2(const std::string& id);
  virtual ~BasicPassword();

  virtual std::string generate(const Seed& seed) const;
  virtual std::string prepare(const std::string& base) const;

  std::size_t length;
  std::string postfix;
  std::unordered_set<char> bannedChars;
  char fill;
};
} // namespace genpass
