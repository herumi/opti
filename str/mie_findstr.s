.text
.global mie_findStr
.global mie_findCaseStr

mie_findStr:
    mov    %rdx, %r8
    mov    %rcx, %rax
    mov    %rsi, %rdx
    sub    %rdi, %rdx
    push   %r12
    movdqu (%r8), %xmm0
.lp0:
    pcmpestri $0xc, (%rdi), %xmm0
    lea    0x10(%rdi), %rdi
    lea    -0x10(%rdx), %rdx
    ja     .lp0
    jae    .notFound0
    lea    -0x10(%rdi, %rcx, 1), %rdi
    sub    %rcx, %rdx
    add    $0x10, %rdx
    mov    %rdi, %r9
    mov    %r8, %r10
    mov    %rax, %r11
    mov    %rdx, %r12
.tailCmp0:
    movdqu (%r10), %xmm1
    pcmpestri $0xc, (%r9), %xmm1
    jno    .next0
    js     .found0
    add    $0x10, %r9
    add    $0x10, %r10
    sub    $0x10, %rax
    sub    $0x10, %rdx
    jmp    .tailCmp0
.next0:
    add    $0x1, %rdi
    mov    %r11, %rax
    lea    -0x1(%r12), %rdx
    jmp    .lp0
.notFound0:
    mov    %rsi, %rax
    jmp    .exit0
.found0:
    mov    %rdi, %rax
.exit0:
    pop    %r12
    ret

mie_findCaseStr:
    mov    %rdx, %r8
    mov    %rcx, %rax
    mov    %rsi, %rdx
    sub    %rdi, %rdx
    push   %r12
    mov    $0x40404040, %r9d
    movd   %r9d, %xmm4
    pshufd $0x0, %xmm4, %xmm4
    mov    $0x5b5b5b5b, %r9d
    movd   %r9d, %xmm5
    pshufd $0x0, %xmm5, %xmm5
    mov    $0x20202020, %r9d
    movd   %r9d, %xmm6
    pshufd $0x0, %xmm6, %xmm6
    movdqu (%r8), %xmm0
.lp1:
    movdqu (%rdi), %xmm1
    movdqa %xmm1, %xmm2
    pcmpgtb %xmm4, %xmm2
    movdqa %xmm5, %xmm3
    pcmpgtb %xmm1, %xmm3
    pand   %xmm3, %xmm2
    pand   %xmm6, %xmm2
    paddb  %xmm2, %xmm1
    pcmpestri $0xc, %xmm1, %xmm0
    lea    0x10(%rdi), %rdi
    lea    -0x10(%rdx), %rdx
    ja     .lp1
    jae    .notFound1
    lea    -0x10(%rdi, %rcx, 1), %rdi
    sub    %rcx, %rdx
    add    $0x10, %rdx
    mov    %rdi, %r9
    mov    %r8, %r10
    mov    %rax, %r11
    mov    %rdx, %r12
.tailCmp1:
    movdqu (%r9), %xmm2
    movdqa %xmm2, %xmm1
    pcmpgtb %xmm4, %xmm1
    movdqa %xmm5, %xmm3
    pcmpgtb %xmm2, %xmm3
    pand   %xmm3, %xmm1
    pand   %xmm6, %xmm1
    paddb  %xmm1, %xmm2
    movdqu (%r10), %xmm1
    pcmpestri $0xc, %xmm2, %xmm1
    jno    .next1
    js     .found1
    add    $0x10, %r9
    add    $0x10, %r10
    sub    $0x10, %rax
    sub    $0x10, %rdx
    jmp    .tailCmp1
.next1:
    add    $0x1, %rdi
    mov    %r11, %rax
    lea    -0x1(%r12), %rdx
    jmp   .lp1
.notFound1:
    mov    %rsi, %rax
    jmp    .exit1
.found1:
    mov    %rdi, %rax
.exit1:
    pop    %r12
    ret
 
