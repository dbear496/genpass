#!/usr/bin/env bash

export seed="test seed"
export masterpw="test masterpw"
tmp=$(mktemp -d)
homebak=$tmp/.genpass_bak
export homedir=$tmp/.genpass

passed=0
failed=0
skipped=0

./genpass.sh --home $homebak --create <(echo -n $seed) < <(echo -e "${masterpw}\n${masterpw}") >/dev/null

for test in $(ls tests); do
  echo -n "running test $test..."
  
  if [[ ! -x tests/$test ]]; then
    echo "skip"
    : $((skipped += 1))
    continue
  fi
  
  # make a clean home
  cp -r $homebak $homedir
  
  # run the test
  if msg=$(tests/$test); then
    echo "pass"
    : $((passed += 1))
  else
    echo "fail"
    : $((failed += 1))
  fi
  if [[ -n "$msg" ]]; then
    echo $msg
  fi
  
  # delete the used home
  rm -rf $homedir
done

rm -r $homebak

echo "$passed tests passed"
echo "$failed tests failed"
echo "$skipped tests skipped"

if [[ $failed -eq 0 ]]; then
  :
else
  exit 1
fi
