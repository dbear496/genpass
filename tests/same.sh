#!/usr/bin/env bash

tag="test tag"

genpw=$(./genpass.sh --home $homedir --output /dev/fd/3 3>&1 < <(echo -e "${tag}\ny\n${masterpw}") >/dev/null)

sampw=$(./genpass.sh --home $homedir --output /dev/fd/3 3>&1 < <(echo -e "${tag}\n${masterpw}") >/dev/null)

if [[ $genpw != $sampw ]]; then
  echo "expected '$genpw' == '$sampw'"
  exit 1
fi
