#!/usr/bin/env bash

tag="test tag"
exppw="wJgh0hKXLB/K4YyZWgScRLw5YbAbxdFEUvY9DDGgx4U="

genpw=$(./genpass.sh --home $homedir --output >(cat) < <(echo -e "${tag}\ny\n${masterpw}") >/dev/null)

if [[ $genpw != $exppw ]]; then
  echo "expected '$exppw' but got '$genpw'"
  exit 1
fi
