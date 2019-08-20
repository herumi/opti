@echo off
nasm -f win64 test.asm
cl /EHsc t.cpp test.obj /Od /Zi
sde -- t.exe
