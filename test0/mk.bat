@echo off
nasm -f win64 -D_WIN64 test.asm
cl /EHsc /Ox /DNDEBUG -I ../../cybozulib/include t.cpp test.obj
