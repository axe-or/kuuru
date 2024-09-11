set -eu

mv base base.old.o || true
wget https://github.com/axe-or/base.c/raw/main/base.zip -O base.zip
unzip base.zip
chmod 755 base
rm -r base.old.o
