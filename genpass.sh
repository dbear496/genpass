#!/usr/bin/env bash

saltfile="$HOME/.genpass_salt"
checkfile="$HOME/.genpass_check"

new="n"
clip="n"
print="y"
output=""
help="n"

POSITIONAL_ARGS=()
while [[ $# -gt 0 ]]; do
  case $1 in
    --create)
      new="y"
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

if [ ! -e $saltfile ]; then
  echo "warning: no salt file, creating blank salt file at '$saltfile'"
  touch $saltfile
fi
salt=$(cat $saltfile | base64)

if [[ $new == "y" ]]; then
  echo "WARNING: This will change the generated passwords. To abort, press ctrl-C."
  echo -n "new master password: "
  read -s password
  echo
  echo -n "confirm new master password: "
  read -s conf
  echo
  if [[ $password != $conf ]]; then
    echo "error: passwords do not match"
    exit 1
  fi
  echo -n "${password}${salt}" | openssl sha256 -binary > $checkfile
else
  echo -n "tag: "
  read tag
  echo -n "password: "
  read -s password
  echo
  while ! cmp -s $checkfile <(echo -n "${password}${salt}"  | openssl sha256 -binary); do
    sleep 1
    echo "incorrect password"
    echo -n "password: "
    read -s password
    echo
  done
  
  pass=$(echo -n "${password}${salt}${tag}" | openssl sha256 -binary | base64)
  
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
fi
