/* ---------------------------------------------------------------------- *\
 * src/Genpass.cpp
 * This file is part of GenPass.
 *
 * Copyright (C) 2026       David Bears <dbear4q@gmail.com>
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

template<>
void
Genpass::readConfig<nlohmann::json>(nlohmann::json&& in) {
  bool unknownAlg = false;

  for(const auto& pw : in.at("passwords")) {
    std::string algName = pw.at("algorithm").get<std::string>();

    auto loaderLookup = loaders.find(algName);
    if(loaderLookup == loaders.end()) {
      unknownAlg = true;
      std::println(stderr, "error: unknown algorithm for %s: %s",
        pw.at("id"), algName);
      continue;
    }
    Loader& loader = *loaderLookup;

    Password password = loader(pw);

    if(!passwords.insert({password.id, password}).second) {
      std::println(stderr, "warning: password with this id already exists: %s",
        password.id);
    }
  }

  if(unknownAlg) {
    std::println(stderr,
      "error: Some passwords could not be loaded because there is no loader."
      " This could happen if a plugin is missing.");
  }
}

} // namespace genpass
