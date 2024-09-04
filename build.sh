#/usr/bin/env sh
set -eu

cc='clang++'
cflags='-std=c++17 -O1 -fPIC -Wall -Wextra'
ldflags=''

for f in $(find base -type f -name '*.cpp'); do
    $cc $cflags -c "$f" -o "$f.o" &
done
wait


$cc $cflags yuyu.cpp base/*.o -o yuyu $ldflags