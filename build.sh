#/usr/bin/env sh

mode="$1"
[ -z "$mode" ] \
    && mode="debug"

set -eu

cc='clang++'
cflags='-std=c++17 -fPIC -Wall -Wextra'
ldflags=''
ignoreflags='-Wno-unknown-pragmas'

case "$mode" in 
    "debug") cflags="$cflags -O0 -g -fsanitize=address" ;;
    "release") cflags="$cflags -O2" ;;
esac
set -x

$cc $cflags $ignoreflags -c base/lib.cpp -o base.o
$cc $cflags $ignoreflags yuyu.cpp base.o -o yuyu $ldflags
./yuyu
