#!/usr/bin/env bash

tag="test tag"
newmasterpw="new master password"

befpw=$(./genpass.sh --home $homedir --output /dev/fd/3 3>&1 < <(echo -e "${tag}\ny\n${masterpw}") >/dev/null)

./genpass.sh --home $homedir --change < <(echo -e "${masterpw}\n${newmasterpw}\n${newmasterpw}") >/dev/null

aftpw=$(./genpass.sh --home $homedir --output /dev/fd/3 3>&1 < <(echo -e "${tag}\n${newmasterpw}") >/dev/null)

if [[ $befpw != $aftpw ]]; then
  echo "expected '$genpw' == '$sampw'"
  exit 1
fi
