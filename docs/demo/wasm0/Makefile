all: square.wasm
EMCC=emcc
WABT_PATH=~/soft/wabt/out/clang/Debug/
WAST2WASM=$(WABT_PATH)/wast2wasm
square.wasm: square.wat
	$(WAST2WASM) -v square.wat -o square.wasm
run-server:
	python -m SimpleHTTPServer

clean:
	rm -rf square.wasm
