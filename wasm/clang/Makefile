CXX=clang++-8

CLANG_WASM_OPT=-O3 -DNDEBUG -fPIC -DMCL_SIZEOF_UNIT=8 -DMCL_MAX_BIT_SIZE=384 -DMCL_LLVM_BMI2=0 -DMCL_USE_LLVM=1 -DCYBOZU_DONT_USE_EXCEPTION -DCYBOZU_MINIMUM_EXCEPTION -DCYBOZU_DONT_USE_STRING -DMCL_DONT_USE_CSPRNG -I./include -I./src -I../mcl/include -fno-builtin  --target=wasm32-unknown-unknown-wasm  -Wstrict-prototypes -Wno-unused-parameter -ffreestanding -fno-exceptions -fvisibility=hidden -Wall -Wextra -fno-threadsafe-statics -nodefaultlibs -nostdlib -fno-use-cxa-atexit -fno-unwind-tables -fno-rtti -nostdinc++ -DLLONG_MIN=-0x8000000000000000LL

addsub.wasm: add.o sub.o
	wasm-ld-8 --no-entry --export-dynamic --initial-memory=131072 -o $@ $?
	chmod 644 $@

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CLANG_WASM_OPT)

add.o: add.cpp
sub.o: sub.cpp

dis: addsub.wasm
	llvm-objdump-8 -disassemble addsub.wasm

test: addsub.wasm
	node test.js

clean:
	rm -rf *.o *.wasm
