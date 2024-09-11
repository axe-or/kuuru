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

bin/compiler.o: $(wildcard compiler/*.c compiler/*.h)
	$(CC) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) -c compiler/kuuru.c -o bin/compiler.o

bin/kuuru: main.c bin/compiler.o bin/base.o
	$(CC) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) main.c bin/compiler.o bin/base.o -o bin/kuuru $(LDFLAGS)

clean:
	rm -f bin/*
