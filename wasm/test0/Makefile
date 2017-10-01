all: add.wasm

CFLAGS+=-O3 

add.wasm: add.cpp
	emcc add.cpp $(CFLAGS) -o add.js -s WASM=1 -s "MODULARIZE=1" -s "EXPORTED_FUNCTIONS=['_add', '_str2int', '_int2str', '_int64_t2str', '_str2int64_t']"

clean:
	rm -rf add.js add.wasm
