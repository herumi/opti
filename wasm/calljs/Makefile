all: fill.wasm fill.js

CFLAGS+=-O3 -DNDEBUG -s WASM=1 -s DISABLE_EXCEPTION_CATCHING=0 -s NO_EXIT_RUNTIME=1 -s MODULARIZE=1

fill.wasm: fill.cpp filljs.js
fill.js: fill.cpp filljs.js Makefile
	emcc fill.cpp $(CFLAGS) --pre-js filljs.js -o fill.js
#	emcc fill.cpp $(CFLAGS) -o fill.js

clean:
	rm -rf fill.js fill.wasm
