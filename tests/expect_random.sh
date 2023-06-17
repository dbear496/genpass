#!/usr/bin/env bash

count=20

for i in $(seq $count); do
  tag="tag_$RANDOM"
  
  exppw=$(cat \
    <(echo -n $seed) \
    <(echo -n $tag) \
    <(yes salt | tr -d '\n' | head -c100000000) |
    openssl sha256 --binary | base64
  )
  
  genpw=$(./genpass.sh --home $homedir --output /dev/fd/3 3>&1 < <(echo -e "$tag\ny\n${masterpw}") >/dev/null)
  
  if [[ $genpw != $exppw ]]; then
    echo "for tag '$tag', expected '$exppw' but got '$genpw'"
    exit 1
  fi
  
done
