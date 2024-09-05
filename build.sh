#/usr/bin/env sh
set -eu

cc='clang++'
cflags='-std=c++17 -O1 -fPIC -Wall -Wextra'
ldflags=''

set -x

$cc $cflags -c base/lib.cpp -o base.o
$cc $cflags yuyu.cpp base.o -o yuyu $ldflags
