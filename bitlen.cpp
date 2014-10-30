#include <stdio.h>
#include <string>
#include <math.h>
#include <iostream>
#include <cybozu/bit_operation.hpp>

int bitLen(uint32_t x)
{
	if (x == 0) return 1;
	for (int i = 0; i < 32; i++) {
		if (x < (1u << i)) {
			return i;
		}
	}
	return 32;
}

/*
VC:
	bsr  eax, ecx
	je   .zero
	inc  eax
	ret
.zero:
	mov  eax, 1
	ret

clang:
    movl    $1, %eax
    testl   %edi, %edi
    je  .exit
    bsrl    %edi, %eax
    xorl    $-32, %eax
    addl    $33, %eax
.exit:
    ret

gcc(*1):
    testl   %edi, %edi
    movl    $1, %eax
    je  .L9
    bsrl    %edi, %eax
    addl    $1, %eax
.L9:
    rep
    ret
gcc:
    testl   %edi, %edi
    movl    $1, %eax
    je  .L9
    bsrl    %edi, %edi
    movb    $32, %al
    xorl    $31, %edi
    subl    %edi, %eax
.L9:
    rep
    ret
*/
int bitLen2(uint32_t x)
{
#ifdef _MSC_VER
	unsigned long ret;
	if (_BitScanReverse(&ret, x)) {
		return ret + 1;
	}
	return 1;
#else
	if (x == 0) return 1;
	return 32 - __builtin_clz(x);
//	return (__builtin_clz(x) ^ 0x1f) + 1; // (*1) better for gcc
#endif
}

int bitLen3(uint32_t x)
{
	if (x == 0) return 1;
	return cybozu::bsr(x) + 1;
}

int main()
{
	for (uint32_t i = 0; i < 20; i++) {
		int a = bitLen(i);
		int b = bitLen2(i);
		int c = bitLen3(i);
		printf("%u %d %d %d\n", i, a, b, c);
	}
}
