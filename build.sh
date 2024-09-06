#/usr/bin/env sh

mode="$1"
[ -z "$mode" ] \
    && mode="debug"

set -eu

cc='g++'
iflags='-I.'
cflags='-std=c++17 -fPIC -Wall -Wextra'
ldflags=''
ignoreflags='-Wno-unknown-pragmas'

case "$mode" in 
    "debug") cflags="$cflags -O0 -g" ;;
    "release") cflags="$cflags -O2" ;;
esac
set -x

$cc $iflags $cflags $ignoreflags -c base/lib.cpp -o base.o
$cc $iflags $cflags $ignoreflags -c yuyu.cpp -o yuyu.o
$cc $iflags $cflags $ignoreflags main.cpp yuyu.o base.o -o yuyu $ldflags

./yuyu
