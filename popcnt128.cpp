#include <stdint.h>
#include <cybozu/bit_operation.hpp>

/*
gcc 4.7
    mov rcx, rdx
    mov r8d, 1
    and ecx, 63
    sal r8, cl
    and edx, 64
    lea rax, [r8-1]
    je  .L3
    and rax, rsi
    popcnt  rdx, rax
    mov rax, -1
    and rax, rdi
    popcnt  rax, rax
    add rax, rdx
    ret
    .p2align 4,,10
    .p2align 3
.L3:
    xor edx, edx
    and rax, rdi
    popcnt  rax, rax
    add rax, rdx
    ret

clang 3.3
    movl    $1, %eax
    movb    %dl, %cl
    shlq    %cl, %rax
    decq    %rax
    andl    $64, %edx
    shrl    $6, %edx
    xorl    %ecx, %ecx
    testl   %edx, %edx
    cmovneq %rax, %rcx
    movq    $-1, %rdx
    cmoveq  %rax, %rdx
    andq    %rdi, %rdx
    popcntq %rdx, %rdx
    andq    %rsi, %rcx
    popcntq %rcx, %rax
    addq    %rdx, %rax
    ret

VC2012
	// rcx = b0, rdx = b1, r8 = idx
	mov     r9, rcx
	movzx   ecx, r8b
	mov     r10d, 1
	and     cl, 63   // idx & 63
	shl     r10, cl  // 1 << (idx & 63)
	mov     rcx, -1
	dec     r10      // r10 = mask
	and     r8d, 64  // idx & 64
	mov     rax, r10
	cmovne  rax, rcx // rax = (idx & 64) ? -1 : mask
	xor     ecx, ecx
	and     rax, r9  // b0 & m0
	popcnt  rax, rax // ret = popcnt(b0 & m0)
	test    r8, r8   // id x& 64
	cmovne  rcx, r10 // rcx = (idx & 64) ? mask : 0
	mov     eax, eax
	and     rcx, rdx
	popcnt  rdx, rcx
	mov     edx, edx
	add     rax, rdx
	ret
*/
uint64_t maskAndPopcnt(uint64_t b0, uint64_t b1, uint64_t idx)
{
	const uint64_t mask = (uint64_t(1) << (idx & 63)) - 1;
	uint64_t m0 = (idx & 64) ? -1 : mask;
	uint64_t m1 = (idx & 64) ? mask : 0;
	uint64_t ret = cybozu::popcnt(b0 & m0);
	ret += cybozu::popcnt(b1 & m1);
	return ret;
}

