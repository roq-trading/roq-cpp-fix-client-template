#!/usr/bin/env bash

KERNEL="$(uname -a)"

if [ "$1" == "debug" ]; then
  case "$KERNEL" in
    Linux*)
      PREFIX="gdb --args"
      ;;
    Darwin*)
      PREFIX="lldb --"
      ;;
  esac
  shift 1
else
  PREFIX=
fi

$PREFIX ./roq-cpp-fix-client-template \
  --name "fix-client" \
  --fix_target_comp_id "fix-bridge" \
  --fix_sender_comp_id "fix-client" \
  --fix_username "trader" \
  --fix_password "secret" \
  --service_listen_address 2345 \
  $@
