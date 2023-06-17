#!/usr/bin/env bash

tag1="test tag"
tag2="other tag"

genpw=$(./genpass.sh --home $homedir --output >(cat) < <(echo -e "${tag1}\ny\n${masterpw}") >/dev/null)

difpw=$(./genpass.sh --home $homedir --output >(cat) < <(echo -e "${tag2}\ny\n${masterpw}") >/dev/null)


if [[ $genpw == $difpw ]]; then
  echo "expected '$genpw' != '$difpw'"
  exit 1
fi
