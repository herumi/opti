all: add.wasm

CFLAGS+=-O3 

add.wasm: add.cpp
	emcc add.cpp $(CFLAGS) -o add.js -s WASM=1 -s "MODULARIZE=1" -s "EXPORTED_FUNCTIONS=['_add', '_str2int', '_int2str']"

clean:
	rm -rf add.js add.wasm
