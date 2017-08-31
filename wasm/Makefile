all: mclbn.wasm exported-mcl.txt exported-mcl.js
EMCC=emcc
MCL_PATH=../../mcl/

CFLAGS+=-O3 -I ../../mcl/include -I ../../cybozulib/include

exported-mcl.txt: $(MCL_PATH)/include/mcl/bn.h
	python export_functions.py $< > $@

exported-mcl.js: $(MCL_PATH)/include/mcl/bn.h
	python export_functions.py $< -js mcl > $@

EXPORTED_MCL=$(shell cat exported-mcl.txt)

mclbn.wasm: $(MCL_PATH)src/fp.cpp $(MCL_PATH)src/bn_c256.cpp exported-mcl.txt
	emcc $(MCL_PATH)src/fp.cpp -o mclbn.js $(MCL_PATH)src/bn_c256.cpp $(CFLAGS) -o mclbn.js -s WASM=1 -s "MODULARIZE=1" -s "EXPORTED_FUNCTIONS=[$(EXPORTED_MCL)]" -DMCLBN_FP_UNIT_SIZE=4 -DDISABLE_EXCEPTION_CATCHING=2

run-server:
	python -m SimpleHTTPServer

clean:
	rm -rf mclbn.wasm exported-mcl.txt exported-mcl.js
