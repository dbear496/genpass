#!/usr/bin/bash

# GenPass parameters
confdir="$HOME/.genpass"
seedfile="$confdir/seed"
seedcheckfile="$confdir/seed_check"
tagsfile="$confdir/tags"
saltstr="salt" # changing saltstr or saltsize will change the generated passwords
saltsize=100000000 # one hundred million

# command line defaults
create="n"
change="n"
clip="n"
print="y"
output=""
help="n"

# command line parsing. I got this from stack overflow
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

# help message
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

# create the configuration directory if it doesn't already exist
if ! [ -d $confdir ]; then mkdir -p $confdir; fi

# import seed or change password
if [[ $create == "y" || $change == "y" ]]; then
  if [[ $create == "y" && $change == "y" ]]; then
    echo "genpass: incompatable options --create and --change" >&2
    exit 1
  fi
  if [[ $change == "y" ]]; then
    # get and verify old password
    while
      echo -n "old password: "; read -s oldpassword; echo
      ! openssl aes256 -d -in $seedfile -pass file:<(echo -n $oldpassword) -pbkdf2 2>/dev/null |
        cat - <(yes $saltstr | tr -d '\n' | head -c$saltsize) |
        openssl sha256 -binary |
        cmp -s - $seedcheckfile
    do
      echo "incorrect password, try again"
    done
  elif [ -e $seedfile ]; then
    echo "WARNING: Setting new seed. This will change existing passwords."
  fi
  # get and confirm new password
  while
    echo -n "new password: "; read -s newpassword; echo
    echo -n "confirm new password: "; read -s conf; echo
    [[ "$newpassword" != "$conf" ]]
  do
    echo "passwords do not match, try again"
  done
  
  if [[ $create == "y" ]]; then
    # create a new seed check file from the imported seed
    cat $newseedfile | tee >(
      cat - <(yes $saltstr | tr -d '\n' | head -c$saltsize) |
      openssl sha256 -binary > ${seedcheckfile}.tmp
    )
  else
    # decrypt the seed with the old password
    openssl aes256 -d -in $seedfile -pass file:<(echo -n $oldpassword) -pbkdf2
  fi |
    # encrypt the seed with the new password
    openssl aes256 -e -pass file:<(echo -n $newpassword) -pbkdf2 > ${seedfile}.tmp
  
  # move the new seed to the correct location (overwriting any old seed)
  mv -f "${seedfile}"{.tmp,}
  mv -f "${seedcheckfile}"{.tmp,}
  
  echo "success"
  exit
fi

# make sure the seed and the seed check files exists
if ! [ -e $seedfile -a -e $seedcheckfile ]; then
  echo "genpass: missing seed file or seed check file" >&2
  echo "         please import a seed with the --create option" >&2
  exit 1
fi

# get tag from user
while
  echo -n "tag: "; read tag
  if [ -z "$tag" ]; then
    echo "tag may not be empty"
  elif ! ( [ -f $tagsfile ] && grep -xFq -e "$tag" $tagsfile ); then
    # confirm that the user is making a new tag and it's not a typo
    while
      echo -n "confirm new tag '$tag' (y/n): "; read tagconf
      ! [[ "$tagconf" == "y" || "$tagconf" == "Y" || "$tagconf" == "n" || "$tagconf" == "N" ]]
    do :; done
    if [[ $tagconf == "y" || $tagconf == "Y" ]]; then
      echo $tag >> $tagsfile
    else
      tag=""
    fi
  fi
  [ -z "$tag" ]
do :; done

# get and verify the master password
while
  echo -n "password: "; read -s password; echo
  ! openssl aes256 -d -in $seedfile -pass file:<(echo -n $password) -pbkdf2 2>/dev/null |
    cat - <(yes $saltstr | tr -d '\n' | head -c$saltsize) |
    openssl sha256 -binary |
    cmp -s - $seedcheckfile
do
  echo "incorrect password, try again"
done

# generate the password
pass=$(
  openssl aes256 -d -in $seedfile -pass file:<(echo -n $password) -pbkdf2 |
  cat - <(echo -n $tag) |
  cat - <(yes $saltstr | tr -d '\n' | head -c$saltsize) |
  openssl sha256 -binary |
  openssl base64 -e
)

if [[ $clip == "y" ]]; then
  # copy generated password to the clipboard
  echo -n "$pass" | xclip -selection clipboard
  sleep 0.1
fi
if [[ $print == "y" ]]; then
  # print the generated password to standard output
  echo "$pass"
fi
if [[ $output != "" ]]; then
  # write the generated password to a file
  echo -n "$pass" > $output
fi
