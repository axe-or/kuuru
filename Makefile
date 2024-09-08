CXX := g++
CFLAGS := -O1 -g -fPIC -std=c++17 -Wall -Wextra
INCFLAGS := -I.
LDFLAGS :=
IGNOREFLAGS := -Wno-unknown-pragmas

.PHONY: clean build

build: ./bin bin/kuuru
	@./bin/kuuru

./bin:
	mkdir -p bin

bin/base.o: base/$(wildcard base/*.c base/*.h)
	$(CXX) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) -c base/lib.cpp -o bin/base.o

bin/compiler.o: $(wildcard compiler/*.c compiler/*.h)
	$(CXX) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) -c compiler/kuuru.cpp -o bin/compiler.o

bin/kuuru: main.cpp bin/compiler.o bin/base.o
	$(CXX) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) main.cpp bin/compiler.o bin/base.o -o bin/kuuru $(LDFLAGS)

clean:
	rm -f bin/*
