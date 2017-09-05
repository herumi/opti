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
	%define _p2 rdx
%else
	%define _p1 rdi
	%define _p2 rsi
%endif


%macro _OP 0
	cmpxchg8b [r8]
;	cpuid
%endmacro


segment .text
	align 16
func1:
	push rbx
	mov r8, _p1
	mov r9, _p2
.lp:
	_OP
	dec r9
	jnz .lp
	pop rbx
	ret

	align 16
func2:
	push rbx
	mov r8, _p1
	mov r9, _p2
.lp:
	call call_cmpxchg8b
	dec r9
	jnz .lp
	pop rbx
	ret

	align 16
func3:
	push rbx
	mov r8, _p1
	mov r9, _p2
.lp:
	call call_cmpxchg8b2
	dec r9
	jnz .lp
	pop rbx
	ret

	align 16
func4:
	push rbx
	mov r8, _p1
	mov r9, _p2
.lp:
	call call_cmpxchg8b3
	dec r9
	jnz .lp
	pop rbx
	ret

	align 16
call_cmpxchg8b:
	_OP
	ret

	align 16
call_cmpxchg8b2:
	cmp [rel flag0], byte 0
	jz call_cmpxchg8b
	ret

	align 16
call_cmpxchg8b3:
	cmp [rel flag1], byte 0
	jnz call_cmpxchg8b
	ret
