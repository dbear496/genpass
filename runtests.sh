#!/usr/bin/env bash

export seed="test seed"
export masterpw="test masterpw"
tmp=$(mktemp -d)
homebak=$tmp/.genpass_bak
export homedir=$tmp/.genpass
timeout="10s"

passed=0
failed=0
skipped=0

./genpass.sh --home $homebak --create <(echo -n $seed) < <(echo -e "${masterpw}\n${masterpw}") >/dev/null

for test in $(ls tests); do
  echo -n "running test $test..."
  
  # skip if the test is not executable
  if [[ ! -x tests/$test ]]; then
    echo "skip"
    : $((skipped += 1))
    continue
  fi
  
  # make a clean home
  cp -r $homebak $homedir
  
  # run the test
  if msg=$(timeout $timeout tests/$test); then
    echo "pass"
    : $((passed += 1))
  else
    if [[ $? -eq 124 ]]; then
      echo "fail (timeout)"
    else
      echo "fail"
    fi
    : $((failed += 1))
  fi
  if [[ -n "$msg" ]]; then
    echo $msg
  fi
  
  # delete the used home
  rm -rf $homedir
done

# delete temporary directory
rm -r $tmp

# report
echo "$passed tests passed"
echo "$failed tests failed"
echo "$skipped tests skipped"

if [[ $failed -ne 0 ]]; then
  exit 1
fi
