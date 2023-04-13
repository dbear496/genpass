#!/usr/bin/env bash

confdir="$HOME/.genpass"
seedfile="$confdir/seed"
seedcheckfile="$confdir/seed_check"
pwconfirmstr="00000000"
checkfile="$HOME/.genpass_check"

create="n"
change="n"
clip="n"
print="y"
output=""
help="n"

POSITIONAL_ARGS=()
while [[ $# -gt 0 ]]; do
  case $1 in
    --create)
      create="y"
      newseedfile=$2
      shift # past argument
      shift
      ;;
    --change)
      change="y"
      shift # past argument
      ;;
    -c|--clipboard)
      clip="y"
      print="n"
      shift # past argument
      ;;
    --no-clipboard)
      clip="n"
      shift # past argument
      ;;
    -p|--print)
      print="y"
      clip="n"
      shift # past argument
      ;;
    -n|--no-print)
      print="n"
      shift # past argument
      ;;
    -b|--both)
      print="y"
      clip="y"
      shift # past argument
      ;;
    -o|--output)
      output=$2
      shift # past argument
      shift
      ;;
    -h|--help)
      help="y"
      shift # past argument
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      echo "Unknown option $1"
      exit 1
      POSITIONAL_ARGS+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done
set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

if [[ $help == "y" ]]; then
  echo "genpass [-c | --clipboard] [--no-clipboard] [-p | --print] [-n | --no-print] [-b | --both] [{-o | --output} <file>] [--create] [-h | --help]"
  echo "  -c, --clipboard - copy the password to the clipboard and do not print"
  echo "  --noclipbaord   - do not copy to the clipboard"
  echo "  -p, --print     - print the password to stdout and do not copy to the clipboard (default)"
  echo "  -n, --no-print  - do not print to stdout"
  echo "  -b, --both      - print the password to stdout and copy it to the clipboard"
  echo "  -o <file>"
  echo "  --output <file> - write the password to <file>"
  echo "  --create        - create/change the master password. WARNING: This will change the generated passwords!"
  echo "  -h, --help      - print this help message"
  exit
fi

if [[ $create == "y" || $change == "y" ]]; then
  if [[ $create == "y" && $change == "y" ]]; then
    echo "genpass: incompatable options --create and --change" >&2
    exit 1
  fi
  if [[ $change == "y" ]]; then
    while
      echo -n "old password: "; read -s oldpassword; echo
      ! cmp -s $seedcheckfile <(openssl aes256 -d -in $seedfile -pass file:<(echo -n $oldpassword) -pbkdf2 | openssl sha256 -binary)
    do
      sleep 1
      echo "incorrect, try again"
    done
  fi
  while
    echo -n "new password: "; read -s newpassword; echo
    echo -n "confirm new password: "; read -s conf; echo
    [[ $password != $conf ]]
  do
    echo "passwords do not match, try again"
  done
  
  openssl aes256 -e -in <(if [[ $create == "y" ]]; then cat $newseedfile | tee >(openssl sha256 -binary > ${seedcheckfile}); else openssl aes256 -d -in $seedfile -pass file:<(echo -n $oldpassword) -pbkdf2) -pass file:<(echo -n $newpassword) -pbkdf2 > ${seedfile}.new
  
  mv "${seedfile}"{.new,}
  
  echo "success"
  exit
fi
  
  
while
  echo -n "tag: "; read tag
  [ -z $tag ]
do
  echo "tag may not be empty"
done
while
  echo -n "password: "; read -s password; echo
  ! cmp -s $seedcheckfile <(openssl aes256 -in $seedfile -pass file:<(echo -n $password) -pbkdf2 | openssl sha256 -binary)
do
  sleep 1
  echo "incorrect, try again"
done

pass=$(cat <(openssl aes256 -d -in $seedfile -pass file:<(echo -n $password) -pbkdf2) <(echo -n $tag) | openssl sha256 -binary | openssl base64)

if [[ $clip == "y" ]]; then
  echo -n "$pass" | xclip -selection clipboard
  sleep 0.1
fi
if [[ $print == "y" ]]; then
  echo "$pass"
fi
if [[ $output != "" ]]; then
  echo -n "$pass" > $output
fi
