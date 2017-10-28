all: fill.wasm fill.js

CFLAGS+=-O3 -DNDEBUG

fill.wasm: fill.cpp filljs.js
fill.js: fill.cpp filljs.js
	emcc fill.cpp $(CFLAGS) -s WASM=1 -s "MODULARIZE=1" -s "EXPORTED_FUNCTIONS=['_fill1', '_fill2', '_call0', '_call1', '_call2']" --pre-js filljs.js -o fill.js
#	emcc fill.cpp $(CFLAGS) -s WASM=1 -s "MODULARIZE=1" -s "EXPORTED_FUNCTIONS=['_fill']" -o fill.js

clean:
	rm -rf fill.js fill.wasm
