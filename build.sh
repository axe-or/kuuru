#/usr/bin/env sh
set -eu

cc='g++'
cflags='-std=c++17 -O0 -fPIC -Wall -Wextra'
ldflags=''
ignoreflags='-Wno-unknown-pragmas'

clear
set -x

$cc $cflags $ignoreflags -c base/lib.cpp -o base.o
$cc $cflags $ignoreflags yuyu.cpp base.o -o yuyu $ldflags
./yuyu
