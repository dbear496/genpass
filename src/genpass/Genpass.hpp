


namespace genpass {


class Genpass {
  using Loader = std::function<Password, const nlohmann::json&>;

public:
  Password& getPassword(const std::string& id) const;
  void addPassword(std::unique_ptr<Password>&& password);
  void removePassword(const std::string& id);

  void registerLoader(const std::string& name, Loader loader);

private:
  std::unordered_map<std::string, Loader> loaders;
  std::unordered_map<std::string, std::unique_ptr<Password>> passwords;
};
} // namespace genpass
