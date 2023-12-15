#!/bin/bash

infoMsg() {
    echo -e "\033[32;1m${1}\033[0m"
}

warnMsg() {
    echo -e "\033[31;1m${1}\033[0m"
}

checkMsg() {
    echo -e "\033[33;4m${1}\033[0m"
}

alias lt='ls -lhrt'

if [[ "$(uname)" == "Darwin" ]]; then
    cat /etc/os-release | grep "VERSION"
    BREW_ROOT="$(brew --prefix)"
    export CPATH="$BREW_ROOT/include"
    export LIBRARY_PATH="$BREW_ROOT/lib"
else
    warnMsg "This is NOT an Apple platform!"
    cat /etc/os-release | grep PRETTY | grep NAME
fi
