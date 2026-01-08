/* ---------------------------------------------------------------------- *\
 * src/Password.hpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2025      David Bears <dbear4q@gmail.com>
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

class Password {
public:
  Password(const std::string& id);
  virtual ~Password();

  virtual std::string generate(const Seed& seed) const = 0;

  virtual const std::string& algorithmName() const = 0;
  virtual nlohmann::json serialize() const;

  std::string id;
  int serial;
  std::string note;
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
