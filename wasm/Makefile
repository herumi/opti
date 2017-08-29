all: add.wasm mclbn.wasm
EMCC=emcc
WABT_PATH=~/soft/wabt/out/clang/Debug/
MCL_PATH=../../mcl/
WAST2WASM=$(WABT_PATH)/wast2wasm
#square.wasm: square.wat
#	$(WAST2WASM) -v square.wat -o square.wasm

CFLAGS+=-O3 -I ../../mcl/include -I ../../cybozulib/include
#CFLAGS+=-s WASM=1 -s SIDE_MODULE=1 
add.wasm: add.cpp
	emcc add.cpp $(CFLAGS) -o add.js -s WASM=1 -s "MODULARIZE=1" -s "EXPORTED_FUNCTIONS=['_add', '_str2int']"
#	emcc add.cpp $(CFLAGS) -o add.js -s WASM=1 -s SIDE_MODULE=1 -o add.wasm

mclbn.wasm: $(MCL_PATH)src/fp.cpp $(MCL_PATH)src/bn_c256.cpp
	emcc $(MCL_PATH)src/fp.cpp -o mclbn.js $(MCL_PATH)src/bn_c256.cpp $(CFLAGS) -o mclbn.js -s WASM=1 -s "MODULARIZE=1" -s "EXPORTED_FUNCTIONS=['_mclBn_init', '_mclBn_getOpUnitSize']" -DMCLBN_FP_UNIT_SIZE=4

run_server:
	python -m SimpleHTTPServer

clean:
	rm -rf add.js square.js square.wasm add.wasm
