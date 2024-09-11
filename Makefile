CC := gcc
CFLAGS := -O1 -g -fPIC -std=c11 -Wall -Wextra -Werror=vla
INCFLAGS := -I.
LDFLAGS :=
IGNOREFLAGS := -Wno-unknown-pragmas

.PHONY: clean build

build: ./bin bin/kuuru
	@./bin/kuuru

./bin:
	mkdir -p bin

bin/base.o: $(wildcard base/*.c base/*.h)
	$(CC) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) -c base/base.c -o bin/base.o

bin/kuuru_c.o: $(wildcard kuuru_c/*.c kuuru_c/*.h)
	$(CC) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) -c kuuru_c/kuuru.c -o bin/kuuru_c.o

bin/kuuru: main.c bin/kuuru_c.o bin/base.o
	$(CC) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) main.c bin/kuuru_c.o bin/base.o -o bin/kuuru $(LDFLAGS)

clean:
	rm -f bin/*
