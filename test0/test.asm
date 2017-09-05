default rel

global func1
global func2
global func3
global func4

segment .data
align 16
flag0:
dd 0
flag1:
dd 1

%ifdef _WIN64
	%define _p1 rcx
%else
	%define _p1 rdi
%endif

segment .text
	align 16
func1:
	lea rax, [_p1 + 3]
	ret

	align 16
func2:
	lea rax, [_p1 + 5]
	ret

	align 16
func3:
	cmp [rel flag0], byte 0
	jz func1
	jmp func2

	align 16
func4:
	cmp [rel flag1], byte 0
	jz func1
	jmp func2
