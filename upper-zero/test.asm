global dout
global f_addpd
global f_vaddpd
global f_addpd512
global f_vaddpd512
global f_vaddpd512y

segment .data

din  dq 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0
dout dq 0, 0, 0, 0, 0, 0, 0, 0

segment .text

f_addpd:
	mov		rax, din
	vmovupd	ymm0, [rax]
	addpd	xmm0, xmm0
	mov		rax, dout
	vmovupd	[rax], ymm0
	ret

f_vaddpd:
	mov		rax, din
	vmovupd	ymm0, [rax]
	vaddpd	xmm0, xmm0, xmm0
	mov		rax, dout
	vmovupd	[rax], ymm0
	ret

f_addpd512:
	mov		rax, din
	vmovupd	zmm0, [rax]
	addpd	xmm0, xmm0
	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_vaddpd512:
	mov		rax, din
	vmovupd	zmm0, [rax]
	vaddpd	xmm0, xmm0, xmm0
	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_vaddpd512y:
	mov		rax, din
	vmovupd	zmm0, [rax]
	vaddpd	ymm0, ymm0, ymm0
	mov		rax, dout
	vmovupd	[rax], zmm0
	ret
