#/usr/bin/env sh
set -eu

cc='clang++'
cflags='-std=c++17 -O1 -fPIC -Wall -Wextra'
ldflags=''


$cc $cflags -c base/_lib.cpp -o base.o

$cc $cflags yuyu.cpp base.o -o yuyu $ldflags