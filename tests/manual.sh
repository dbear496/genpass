#!/usr/bin/env bash

tag="test tag"
expseed="test seed"
exppw="wJgh0hKXLB/K4YyZWgScRLw5YbAbxdFEUvY9DDGgx4U="

if [[ $seed != $expseed ]]; then
  echo "the test seed is '$seed', but this test uses an old seed ('$expseed')"
  echo "please update this test to use the new seed"
  exit 1
fi

genpw=$(./genpass.sh --home $homedir --output /dev/fd/3 3>&1 < <(echo -e "${tag}\ny\n${masterpw}") >/dev/null)

if [[ $genpw != $exppw ]]; then
  echo "expected '$exppw' but got '$genpw'"
  exit 1
fi
