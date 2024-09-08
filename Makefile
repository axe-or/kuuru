CXX := g++
CFLAGS := -O0 -g -fPIC -std=c++17 -Wall -Wextra
INCFLAGS := -I.
LDFLAGS :=
IGNOREFLAGS := -Wno-unknown-pragmas

.PHONY: clean build

build: yuyu
	@./yuyu

base.o: base/$(wildcard base/*.c base/*.h)
	$(CXX) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) -c base/lib.cpp -o base.o

compiler.o: $(wildcard compiler/*.c compiler/*.h)
	$(CXX) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) -c compiler/yuyu.cpp -o compiler.o

yuyu: main.cpp compiler.o base.o
	$(CXX) $(IGNOREFLAGS) $(CFLAGS) $(INCFLAGS) main.cpp compiler.o base.o -o yuyu $(LDFLAGS)

clean:
	rm -f *.o yuyu
