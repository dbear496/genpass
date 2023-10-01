#!/usr/bin/env bash

count=20

tags=()
genpws=()
for i in $(seq $count); do
  tag="tag_$RANDOM"
  
  genpw=$($genpass --home $homedir --output /dev/fd/3 3>&1 < <(echo -e "${tag}\ny\n${masterpw}") >/dev/null)
  
  for j in $(seq $((count - 1))); do
    if [[ $genpw == ${genpws[$j]} && $tag != ${tags[$j]} ]]; then
      echo "expected '$genpw' != '${genpws[$j]}' for tags '$tag' and '${tags[$j]}'"
      exit 1
    fi
  done
  
  tags+=("$tag")
  genpws+=("$genpw")
done
