global dout

global f_movsd
global f_movsd_mem
global f_vmovsd
global f_vmovsd_mem
global f_movlpd
global f_vmovlpd
global f_addsd
global f_addpd
global f_vaddsd
global f_vaddpd
global f_vaddpd_y
global f_vaddpd_k

segment .data

din  dq 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0
dout dq 0, 0, 0, 0, 0, 0, 0, 0

segment .text

f_movsd:
	mov		rax, din
	vmovupd	zmm0, [rax]

	movsd	xmm0, xmm0

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_movsd_mem:
	mov		rax, din
	vmovupd	zmm0, [rax]

	movsd	xmm0, [rax]

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_vmovsd:
	mov		rax, din
	vmovupd	zmm0, [rax]

	vmovsd	xmm0, xmm0, xmm0

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_vmovsd_mem:
	mov		rax, din
	vmovupd	zmm0, [rax]

	vmovsd	xmm0, [rax]

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_movlpd:
	mov		rax, din
	vmovupd	zmm0, [rax]

	movlpd	xmm0, [rax]

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_vmovlpd:
	mov		rax, din
	vmovupd	zmm0, [rax]

	vmovlpd	xmm0, xmm0, [rax]

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_addsd:
	mov		rax, din
	vmovupd	zmm0, [rax]

	addsd	xmm0, xmm0

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_addpd:
	mov		rax, din
	vmovupd	zmm0, [rax]

	addpd	xmm0, xmm0

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_vaddsd:
	mov		rax, din
	vmovupd	zmm0, [rax]

	vaddsd	xmm0, xmm0, xmm0

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_vaddpd:
	mov		rax, din
	vmovupd	zmm0, [rax]

	vaddpd	xmm0, xmm0, xmm0

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_vaddpd_y:
	mov		rax, din
	vmovupd	zmm0, [rax]

	vaddpd	ymm0, ymm0, ymm0

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret

f_vaddpd_k:
	mov		rax, din
	vmovupd	zmm0, [rax]

	vaddpd	zmm0, zmm0, zmm0

	mov		rax, dout
	vmovupd	[rax], zmm0
	ret
