all: add.wasm square.wasm
EMCC=emcc
WABT_PATH=~/soft/wabt/out/clang/Debug/
WAST2WASM=$(WABT_PATH)/wast2wasm
square.wasm: square.wat
	$(WAST2WASM) -v square.wat -o square.wasm

CFLAGS+=-O3 -I ../../mcl/include -I ../../cybozulib/include
#CFLAGS+=-s WASM=1 -s SIDE_MODULE=1 
add.wasm: add.cpp
#	emcc add.cpp $(CFLAGS) -o add.js -s WASM=1 -s "MODULARIZE=1" -s "EXPORTED_FUNCTIONS=['_add', '_str2int']"
	emcc add.cpp $(CFLAGS) -o add.js -s WASM=1 -s SIDE_MODULE=1 -o add.wasm

run_server:
	python -m SimpleHTTPServer

clean:
	rm -rf add.js square.js square.wasm add.wasm
