#/usr/bin/env sh
set -eu

cc='clang++'
cflags='-std=c++17 -O1 -fPIC -Wall -Wextra'
ldflags=''

$cc $cflags yuyu.cpp -o yuyu $ldflags