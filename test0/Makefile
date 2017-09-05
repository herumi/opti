all: test0
test.o: test.asm
	nasm -felf64 test.asm

test0: test.o t.cpp
	g++ -Ofast -I../../cybozulib/include t.cpp test.o

clean:
	rm -rf test0 test.o t.o
