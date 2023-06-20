---
marp: true
title: SIMD�Ꭾꡎ�墪���������Ꭾ��ꎣ�掾�
theme: default
paginate: true
style: |
  section {
    justify-content: start;
    background-color: white;
    padding: 0px;
    background: linear-gradient(180deg,#E7FFE7 10%, #008080 10%,#008080 10.5%, white 10.5%, white 100%);
  }
  section h1 {
    margin-bottom: 0px;
    margine: 0px;
    padding-left: 10px;
    padding-top: 5px;
    padding-bottom: 3px;
    background-image: url(images/cybozulabs400.png);
    background-size: 15%;
    background-repeat: no-repeat;
    background-position: right 5px top 5px;
  }
  section.title {
    font-size: 200%;
    padding-left: 0px;
    padding-top: 0px;
    padding-bottom: 0px;
    background: linear-gradient(180deg,#E7FFE7 49%, #008080 49%,#008080 50%, white 50%, white 100%);
    text-align: center;
    justify-content: center;
  }
  section h2 {
    padding-left: 10px;
    padding-top: 0px;
    padding-bottom: 0px;
  }
  section h3 {
    padding-left: 30px;
    padding-top: 0px;
    padding-bottom: 0px;
  }
  section ul {
    margin: 0px;
  }
  section::after {
   font-size: 70%;
   content: attr(data-marpit-pagination) " / " attr(data-marpit-pagination-total);
  }
  table {margin-left: auto;margin-right: auto;table-layout: fixed;width: 90%;display:table;}
  thead th {text-align: center !important;}
  thead tr {background: #eaeaea;}
  tbody tr:nth-child(2n+1) {text-align: center !important;background: #fff;}
  tbody tr:nth-child(2n) {text-align: center !important;background: #eeeeee;}
  section .emp { color: red; }
  section pre {
    margin-bottom: 0px;
  }
---
<!--
headingDivider: 1
-->

<!--
_class: title
-->
# SIMD�Ꭾꡎ�墪���������Ꭾ��ꎣ�掾�
<br>
<br>
�⎵�⎤���夨�⎺�㎻�㎩�� ����콶�ȡ
<br>
�鎮�鎧�玭������鲻�ˢ���2023��� 2023/6/29

# 쨤ꎦ�
## ����쮎�
1. ���ұ
1. �玪��夤�⎻�㎳���妫
1. SIMD
1. x64 CPU
1. �㎬�⎤��妵�⎷�Ꭸ�⎹�㎫�㎼�������
1. FMA
1. exp(x)
1. log(x)
1. ���夻�⎯�㎬�⎸�⎹�⎿

# ���ұ
## �䎪��墡�㎩�⎤���妫�㎪�����㎼�㎫�Ꭿ촎�곎�����
- ���������玪��墩����Ҽ��������ꎦ�墱�Ꭺ��
  - ![width:500px](images/20230607-163615.png)
  - [����鎮�墭������뎫���夤�㎫�⎴�㎪�⎺�㎠����(https://www.docswell.com/s/2300203199/ZQ81DV-2023-05-22-102059#p20)�������Ȫ
- ��倢�������Ꭾ�⎳�㎼�����⎢�⎻�㎳���妬���񎼰掻��躵�SM���墩��ꎿ������������Ꭸ��玤����Ꭾ��掺�玮�
  - 견�ڧ������������墡��ꮬ��������Ҽ�Ꭻ������������������Ꭺ��
  - ASM/SIMD���鲻�Ꭻx64�ᎮAVX-512������轶����

# �玪��夤�⎻�㎳���妫
## [xbyak](https://github.com/herumi/xbyak) : x64���Ꭾ�������Ꭺ�⎳�㎼��������������C++�㎩�⎤���妫�㎪
  - Intel [oneAPI(oneDNN)](https://github.com/oneapi-src/oneDNN)�ᎮCPU�⎨�㎳�⎸�㎳�Ꭷ�莩��������Ꭶ����
  - oneDNN�ᎯPyTorch�⧵ensorFlow�Ꭺ�Ꭹ�ᎮIntel��墩迎������Ꭶ����
  - [�������妮�����Ꭻ�Ꭺ�Ꭳ���倸鎬��4��掻��Xeon SP��倢耎��㎽���掸�墰�펵�ᎯAMX�Ꭸ4�Ꭴ�Ꭾ�⎢�⎯�⎻�㎩�㎬�㎼�⎿](https://pc.watch.impress.co.jp/docs/column/ubiq/1469146.html)
  ![width:380px](https://asset.watch.impress.co.jp/img/pcw/docs/1469/146/011_o.jpg) pc.watch.impress.co.jp�������Ȫ
  - �⎹�㎼���妾�⎳�㎳���妧�㎼�⎿걸玲�����Ꭾ[xbyak_aarch64](https://github.com/fujitsu/xbyak_aarch64)������
## [s_xbyak](https://github.com/herumi/s_xbyak) : Python�Ꭷxbyak�㎩�⎤�⎯�Ꭺ��ꎿ����墩����x64���Ꭾ�������Ꭺ�⎢�⎻�㎳���妫
  - 轴����Ꭿ���墣�����㎡�⎤�㎳��ꎩ����Ꭿ[Xbyak�㎩�⎤�⎯�Ꭺx64��������ASM�������妾�㎫s_xbyak](https://zenn.dev/herumi/articles/s_xbyak-assembler-1)
# SIMD
## 躀�Ꭴ�Ꭾ��轎��Ꭷ��ʲ�Ꭾ��妾�⎿��������墭�玦������
- AVX-512�Ꭿ512�������墰ZMM�㎬�⎸�⎹�⎿��32�����Ꭴ
  zmm0, zmm1, ..., zmm31
- �����������
  - 8���������64���, 16���������32���, 32���������16���, 64���������8��貳����
- 췎����现��ʲ�⎹�������
  - 32�������(float)��16���, 64�������(double)��8��貳����
  - Sapphire Rapids�⧨ranite Rapids�Ꭿ躀�㎨16���������32��墪���墨�鎱���¿轎���夷���妾��
    - [Granite Rapids/Sierra Forest�ᎮAMX/AVX�Ꭾ������轎�](https://zenn.dev/herumi/articles/granite-rapids-sierra-forest)
## ꡎ������Ꭾ꿎�
- op z, x, y # op�Ꭿ��轎�, z, x, y�Ꭿ�㎬�⎸�⎹�⎿��妣�㎢�㎪�������
- z �� op(x, y) # x�Ꭸy�Ꭾop�Ꭾ����������Ꭻ轎��厥�����
# float���墰�ꎠ
## vaddps(z, x, y)
- z, x, y�ᎯZMM�㎬�⎸�⎹�⎿
- x�Ꭾi���������Ꭾfloat���墰�鎴���� $x_i$ �Ꭸ�����
- $z_i = x_i + y_i$ for $i = 0, \dots, 15$
- ps�Ꭿpacked single precision(float)�Ꭾ���µ

x|$x_0$|$x_1$|$x_2$|...|$x_{15}$
-|-|-|-|-|-
+|+|+|+|...|+
y|$y_0$|$y_1$|$y_2$|...|$y_{15}$
=|=|=|=|...|=
z|$z_0$|$z_1$|$z_2$|...|$z_{15}$

# 칎���玭�墰��
## 췎����现��ʲ�⎹�������
  - v+��轎�+{pd, ps}
  - ps : float
  - pd : double (double precision)
## �����������
  - vp + ��轎� + {b, w, d, q}
  - b : byte (8��������ʶ����)
  - w : word (16��������ʶ����)
  - d : dword (32��������ʶ����)
  - q : qword (64��������ʶ����)

# SIMD���掽����
## ������玣���Ꭾ�ꎠ���̤����
```cpp
void addc(float *z, const float *x, const float *y, size_t n) {
  for (size_t i = 0; i < n; i++) {
    z[i] = x[i] + y[i];
  }
}
```
## ����墭�����������Ꭾ轎�갚
- n�Ꭿ쯎��Ꭷ16�Ꭾ���ʲ�Ꭸ�����
- x[0..n], y[0..n], z[0..n]�Ꭿ�����Ꭻ�⎪�㎼���妾�㎩�������墨��墬��
  - restrict�Ꭸ�����
- 64���夦��玢�����Ꭻ�⎢�㎩�⎤�㎳������Ꭶ����������Ꭸ������Ꮎ��������

# SIMD(AVX-512)�Ꭷ�Ꭾ��ꎣ�掾�
## s_xbyak�Ꭻ������ꎣ�掾�
```python
with FuncProc('add_avx512'): # add_avx512�Ꭸ���������������掽���
  with StackFrame(4, vNum=1, vType=T_ZMM) as sf: # ���������Ꭾ���ʲ�Ꭿ4���. ZMM�㎬�⎸�⎹�⎿��1��掽����
    pz = sf.p[0] # 1���������Ꭾ���ʲ�Ꭿz�Ꮈ�Ꭾ���夦�㎳�⎿���������妮�⎸�⎹�⎿���
    px = sf.p[1] # 2���������Ꭾ���ʲ�Ꭿx�Ꮈ�Ꭾ���夦�㎳�⎿
    py = sf.p[2] # 3���������Ꭾ���ʲ�Ꭿy�Ꮈ�Ꭾ���夦�㎳�⎿
    n = sf.p[3]  # 4���������Ꭾ���ʲ�Ꭿn
    lpL = Label() # �㎫�㎼����Ȫ�㎩���妭

    L(lpL) # �㎫�㎼����Ȫ�㎩���妭�Ꭾ��鎾��
    vmovups(zmm0, ptr(px)) # zmm0 = *px
    vaddps(zmm0, zmm0, ptr(py)) # zmm0 += *py as float
    vmovups(ptr(pz), zmm0) # *pz = zmm0
    add(px, 64) # px += 64
    add(py, 64) # py += 64
    add(pz, 64) # pz += 64
    sub(n, 16) # n -= 16
    jnz(lpL) # n != 0 �Ꭺ���pL�Ꭻ�莻�Ꭳ�Ꭶ���ꎿ���
```
# x64 CPU�Ꭾꡎ�
## 쳼�Ȫ�㎬�⎸�⎹�⎿
- 64��������ʶ���� : rax, rbx, rcx, rdx, rsi, rdi, rbp, r8-r15
- 32��������ʶ���� : eax, ebx, ecx, edx, esi, edi, ebp, r8d-r15d
  - 64�������妮�⎸�⎹�⎿�Ꭾ躶掽�32���������迎���
```
|63        32|31  16|15  8|7   0|
|           rax                 |
|            |     eax          |
|            |      |   ax      |
|            |      | ah | al   |
```
- �⎹�⎿��失�㎬�⎸�⎹�⎿ : rsp���妯�㎼�⎫�㎫ꦲ�ʲ�Ꭺ�Ꭹ�����������⎹�⎿��失���ޡ��������񎼉
## ���墰��墰�㎬�⎸�⎹�⎿
- ���妫�⎰�㎬�⎸�⎹�⎿EFLAGS����mp�Ꭺ�Ꭹ�Ꭾ��ꎼ�鎵������窧��妮�⎸�⎹�⎿ - ���ꎿ�����
- 췎����现��ʲ�⎹�������墰FPU������������

# ꡎ�������轎�
## Intel ASM�Ꭾꡎ������Ꭾ꿎�
- op x, y # x �� op(x, y)
  - x����Ꭿ�㎬�⎸�⎹�⎿��妣�㎢�㎪�������
  - x�Ꭸy�Ꭾop�Ꭾ��掽�鎵��������Ꭻ轎��厥�������
## mov x, y # x �� y
- mov(x, ptr(y)) �Ꭿ `x = *(uint64_t*)y;`�Ꭾ���µ������64�������墰ꢎ���񎼉
- x�ᷴIMD�㎬�⎸�⎹�⎿�Ꭾ�Ꭸ��墱movups�Ꭺ�Ꭹ���掽����
## ��ꎡ�莼�鎮�(add, sub, ...)��ꎫ�����鎮�(and, xor, or, ...)
- add x, y # x �� x + y
- and x, y # x �� x & y
# ����轎���玲�
## ��ꎼ� : cmp x, y
- x - y�Ꭾ�������æFLAGS�Ꭻ�⎻�����������莼�鎮�鎵�����Ꭿx�Ꭻ���Т���墬��
  - ZF(Zero Flag) : x == y�Ꭺ��1
  - CF(Carry Flag) : x < y�Ꭺ��1 (unsigned)
- cf. sub x, y�Ꭿ��鎮�鎵��������Ꭻ���Т�������񎼯� �� x - y���
## ��夻�� : test x, y
- x & y�Ꭾ�������æFLAGS�Ꭻ�⎻�����������莼�鎮�鎵�����Ꭿx�Ꭻ���Т������Ꭺ��
  - ZF : x & y == 0�Ꭺ��1
- and x, y�Ꭿ����������æFLAGS�Ꭻ�⎻�������墨x�Ꭻ���Т�������񎼯� �� x & y���
## ���墰��墰��轎�
- �����Ꭾ��鎮�墱EFLAGS���玤��ֶ�������掸���Ꭻ迎�������墰�Ꭿ躴ꎨ�2��

# ���妫�⎰�Ꭻꡎ��Ꭵ��箨���
## �䎡����轎��⎸�㎣�㎳��
- jmp label : ��墦�Ꭷ���abel�Ꭻ�⎸�㎣�㎳��
## ����轎��⎸�㎣�㎳��
- j<�Ꭺ���墪��> label : <�Ꭺ���墪��>������鎫�墨�Ꮀlabel�Ꭻ�⎸�㎣�㎳��
- je : ZF=1���鎵������鎭�����, �������夾�㎭���墬��夺�㎣�㎳��
- jg : �������Ꭷꦎ�������墲�⎸�㎣�㎳��
- ja : ���䎡���墩ꦎ�������墲�⎸�㎣�㎳��
- ��墭jge, jne, jle�Ꭺ�Ꭹ�Ꭺ�Ꭹ
## ����轎�mov
- cmovz x, y : ZF=1�Ꭺ���ov x, y �Ꭺ�Ꭹ����轎��⎸�㎣�㎳���墪������������������

# x64 Windows躴墰MASM���玺��
## -m masm�⎪���夹�㎧�㎳�Ꭾ�����(*.asm���夣�⎤�㎫)
```asm
_text$x segment align(64) execute # 64���夦��玢�����⎢�㎩�⎤�㎳��㎽�Ꭺ��ꎡ�羱�㎽�Ꭺ�⎻�⎰�㎡�㎳����갚
add_avx512 proc export # ���������Ꭾꩶ����
@L1:
vmovups zmm0, zmmword ptr [rdx] # 2���������Ꭾ���ʲ�Ꭿrdx
vaddps zmm0, zmm0, zmmword ptr [r8] # 3���������Ꭾ���ʲ�Ꭿr8
vmovups zmmword ptr [rcx], zmm0 # 1���������Ꭾ���ʲ�Ꭿrcx
add rdx, 64
add r8, 64
add rcx, 64
sub r9, 16
jnz @L1
ret
add_avx512 endp # ���������Ꭾ����
_text$x ends # �⎻�⎰�㎡�㎳��墰����
end # ���夣�⎤�㎫�Ꭾ����
```
# ���Ꮃ�玺���ꎦ�鎴�
## C����ASM����¾�Ꮃ�玺���玠�����
- ���ʲ�Ꭻ�鎲��玽�墨������妮�⎸�⎹�⎿��掿�玭����Ꮉ��妮�⎸�⎹�⎿������
- ���������Ꭾ���ʲ����ʶ�������妽���夦�㎳�⎿���4�����Ꭷ�Ꭾ���񎼉
  - Windows : rcx, rdx, r8, r9
  - Linux : rdi, rsi, rdx, rcx
- ���Ꮃ�玺���������̤������墩���玭����Ꮉ��妮�⎸�⎹�⎿
  - Windows : rbx, rbp, rdi, rsi, r12-r15, rsp
  - Linux : rbx, rbp, r12-r15, rsp
- �����������墰����������Ꭿrax�Ꭻ�厥��
- 𫎳�
  - [Windows x64 �⎽������⎦�⎧�⎢�Ꭾ���Ꮃ�玺���ꎦ�鎴��(https://docs.microsoft.com/ja-jp/cpp/build/x64-software-conventions?view=msvc-160)
  - [x64 Linux System V Application Binary Interface](https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf)

# x64 Linux/macOS躴墰GAS���玺��
## -m gas�⎪���夹�㎧�㎳�Ꭾ�����(*.S���夣�⎤�㎫)
```asm
  ... ����
#endif
.global PRE(add_avx512) # PRE(add_avx512)�Ꭾ�⎷�㎳���妭���玤�릪������骩��㎽�Ꭻ�����
PRE(add_avx512): # PRE(add_avx512)�Ꭾ�⎷�㎳���妭���玮�鎾�������(PRE�ᎯmacOS�Ꭷ�⎢�㎳���㎼�⎹�⎳�⎢���掻�掸�����)
.L1:
vmovups (%rsi), %zmm0 # 2���������Ꭾ���ʲ�Ꭿrsi
vaddps (%rdx), %zmm0, %zmm0 # 3���������Ꭾ���ʲ�Ꭿrdx
vmovups %zmm0, (%rdi) # 1���������Ꭾ���ʲ�Ꭿrdi
add $64, %rsi
add $64, %rdx
add $64, %rdi
sub $16, %rcx
jnz .L1
ret
SIZE(add_avx512) # Linux��GAS�Ꭷ���������Ꭾ�⎵�⎤�⎺���ꎨ��������
```
- Linux�ᎮGNU assembler�Ꭿ���ʲ�Ꭾ�玺�����墭�Ꭺ��񎼰鎴����墬깎������Ꭿ[쩶���깎�����](https://zenn.dev/herumi/articles/s_xbyak-assembler-2)��骩���
- Windows MASM�ᎨLinux GAS�Ꭾ����������������Ꭶ����
# �㎬�⎤��妵�⎷�Ꭸ�⎹�㎫�㎼�������
## CPU�Ꭻ������妮�⎤��妵�⎷
- ��轎����Ҽ�����墨�������墰��ꎡ���갸掺��������Ꭷ�Ꭾ���������失�㎭��失�⎵�⎤�⎯�㎫ : 轎�躵�lk�Ꭸ�������
- �㎬�⎤��妵�⎷��4�Ꭺ��玮�ꎡ�����墨����4clk���墭��������簫����㎽
 ![width:1000px](images/20230613-175001.png)
## Intel�Ꭻ����CPU�Ꭾ�⎹�㎫�㎼�������
- ������轎����鎶����Ꭶ���������墪��墭���墦clk
  - ���������Ꭾ�⎹�㎫�㎼�������墱���掽���������������Ꭻ�玦��墩������墢��
- Intel�Ꭾ�⎹�㎫�㎼�������墱���ʲ(Reciprocal)�⎹�㎫�㎼�������墪����������Ꭸ������񎼰�����簦�������

# �⎹�㎫�㎼�������墰���
## FMA(Fused Multiply -Add)
- $x \rightarrow x + yz$ ���ꎨ�鎮������¿轎�����ntel�Ꭷ�Ꭿvfmadd231ps���
- �㎬�⎤��妵�⎷4 �⎹�㎫�㎼�������0.5�Ꭷ��轎���玣����掾�玭��̤������Ꭺ��玠�����
 ![width:1000px](images/20230613-175628.png)
- �⎹�㎫�㎼�������0.5�Ꭺ�Ꭾ�Ꭷ1clk���墪�Ꭻ2��轎������墩����
- ���玭��̤������䎡��墬��莬���Ꭾ��轎��Ꭿ1clk���墭�����墩����
  - ��������簫���Ꭷ�����Ꭾ�Ꭿ4clk���墭�Ꭺ��
- cf. [�1��� �鎮�鎧�玭������鲻�ˢ���2023����(https://www.docswell.com/s/2300203199/KJLL2G-2023-04-12-102338#p41)

# FMA�Ꭾ�⎹�㎫�㎼���������������玮�뎨�
## ��뎨�夵�㎼��
- �㎫�㎼������Ꭻ�����Ꭻ���玭��̤���墰�䎡�᧧MA���ꎤ��ʲ��掸���Ꮉ�Ꭶ���玺����掾�������
```python
def gen_func(n):
  with FuncProc(f'func{n}'):
    with StackFrame(1, vNum=n+1, vType=T_ZMM) as sf:
      c = sf.p[0]
      lp = Label()
      for i in range(n):
        vxorps(Zmm(i), Zmm(i), Zmm(i)) # ZMM�㎬�⎸�⎹�⎿�⎯�㎪�⎢
      align(32)
      L(lp)
      for i in range(n):
        vfmadd231ps(Zmm(i), Zmm(i), Zmm(i)) # n��墰FMA����Ҽ�
      sub(c, 1)
      jnz(lp) # c���妭�㎼�������
```
- �⎳�㎼��窪��墱[fma](https://github.com/herumi/misc/tree/main/fma)
- Xeon Platinum 8280 Turbo Boost off�Ꭷ캎�갚

# ������⎳�㎼��
## n=1
```asm
@L1:
vfmadd231ps zmm0, zmm0, zmm0
sub rcx, 1
jnz @L1
```

## n=4 �Ꭾ�㎫�㎼���夤�㎳�㎭�㎼�㎫
```asm
@L4:
vfmadd231ps zmm0, zmm0, zmm0
vfmadd231ps zmm1, zmm1, zmm1
vfmadd231ps zmm2, zmm2, zmm2
vfmadd231ps zmm3, zmm3, zmm3
sub rcx, 1
jnz @L4
```

# ��뎨�鎵����
## n���玢������墡�Ꭸ��墰1�㎫�㎼�����������Ꭾ�玦��������

n|1|2|3|4|5|6|7|8|9|10
-|-|-|-|-|-|-|-|-|-|-
clk|4.1|4.0|4.0|4.0|4.0|4.0|4.0|4.0|4.5|5.0|5.50
- n=8�Ꮎ�Ꭷ�Ꭾ�㎫�㎼���夤�㎳�㎭�㎼�㎫�Ꭿ��������玤������墬��񎼰夻�㎫�㎼�������0.5�Ꭷ�玦��墩��墨�������
- FMA�Ꭾ�������Ҽ�墰췢��
 ![width:1000px](images/20230614-095612.png)

# FMA�Ꭾ�莩�����
## ��뎠�玼�墰��掾��
  - $n$ 쮎���뎠�玼� $f(x) = a_0 + a_1x + a_2x^2 + \cdots + a_nx^n$ �Ꭾ $x=x_0$ �Ꭻ������瀎����ꎨ�鎮�����
## $n=4$ �Ꭾ�Ꭸ��墰쳤������
  - $f(x)= a_0 + x(a_1 + x(a_2 + x(a_3 + x a_4)))$ �Ꭸꦲ玽�������
$t \leftarrow a_4$ �Ꭸ���墨���������쳤����
1. $t \leftarrow a_3 + x t$
2. $t \leftarrow a_2 + x t$
3. $t \leftarrow a_1 + x t$
4. $t \leftarrow a_0 + x t$
- �����������Ꭾ�鎮�墭FMA���簫�������
- ������뎠�玼�墭걎��������ʲ�Ꭾ $x_i$ �Ꭻ������掾���Ꭿ�㎫�㎼���夤�㎳�㎭�㎼�㎫���墨�㎬�⎤��妵�⎷����Ԣ��

# ����ʲ�Ꭾ���������쵕
## ����ʲ���窪�Ꭶ�㎬�⎸�⎹�⎿�Ꭻ����������
- �㎬�⎸�⎹�⎿��ꎶ����墬��墬��羱�㎽����������
- ��ꎣ��������Ꭺ��妮�⎸�⎹�⎿�Ꭻ���墨�����Ꭾ������
## ����ʲ���妣�㎢�㎪�Ꭻ����������
- �㎡�㎢�㎪�ᎫSIMD��掸���Ꮉ�Ꭶ�����Ꭶ𬎭��
- �㎡�㎢�㎪�Ꭻ1�����Ꭰ�������墨SIMD�㎬�⎸�⎹�⎿�Ꭻ���ʲ��墢���玱���������������㎭�㎼��夯�㎣�⎹��񎼉
![width:800px](images/20230614-113346.png)

# ���妯�㎼��夯�㎣�⎹��墰����쵕
## ���妯�㎼��夯�㎣�⎹��騣���墰FMA
```python
# coeff
#   |c0|c0|c0|...|c0|c1|c1|...
vfmadd231ps(zmm0, zmm1, ptr(coeff)) # coeff����16�����Ꭾ����float���ꎪ���Ꮏ�����ᎿFMA
```
## ���妯�㎼��夯�㎣�⎹���¿轎�vbroadcastss
```python
# coeff
#   |c0|c1|c2|...
vbroadcastss(zmm2, ptr(coeff)) # coeff����1�����Ꭾfloat���ꎪ���Ꮏ�����Ꮏzmm2�Ꭻ����������
vfmadd231ps(zmm0, zmm1, zmm2)
```
## FMA+ptr_b������㎭�㎼��夯�㎣�⎹�����㎩�⎰���
```python
vfmadd231ps(zmm0, zmm1, ptr_b(coeff)) # coeff����1�����Ꭾfloat���ꎪ���Ꮏ�����Ꮏ��������墨FMA
```

# ��뎨�鎵����
## �㎫�㎼���玱�������墡�Ꭸ��墰�����������Ꭾ�玦��������

N|1|2|3|4|5|6|7|8|9
-|-|-|-|-|-|-|-|-|-
�厨�㎨�㎬�⎸�⎹�⎿|4.13|4.02|4.00|4.00|4.00|4.00|4.00|4.00|4.57
broadcast|4.13|4.01|4.00|4.00|4.00|4.06|4.22|4.74|5.25
ptr_b|4.14|4.01|4.00|4.00|4.00|4.00|4.06|4.38|4.97

- �厨�㎨�㎬�⎸�⎹�⎿�Ꭺ�Ɐ=8������
- �㎡�㎢�㎪����𬎭�Ꮏ�Ꭺ�����Ꭺ���tr_b���掽����墰������

# exp(x)�Ꭾ���掼���鎮�
## �⎢�㎫�⎴�㎪�⎺�㎠
- $\exp(x) = e^x$ ��2���夯�Ꭾ꿎��Ꭻ�����
  - $e^x=2^{x\log_2 e} = 2^y.$ ���$y=x \log_2(e)$���
- $y$ ����ʶ���� $n$ �Ꭸ겾�ʲ�㎨�� $a$($|a|\le 1/2$)�Ꭻ��粴�����
  - $y = n + a$ �Ꭸ������Ꭸ $e^x=2^y = 2^n \times 2^a.$
## $2^n$ �Ꭾ�鎮�
- $n$ ���ʶ�����Ꭺ�Ꭾ�Ꭷ�������莼�鎮�墩�玦��墩����
## $2^a$($|a|\le 1/2$)�Ꭾ�鎮�
- float�Ꭾ�꼎� `1b-23`=$2^{-23}$ �Ꭻ������Ꭺ�����Ꭷ�㎭�㎼�㎩�㎳����������
  - $e^x=1+x+x^2/2!+x^3/3!+\cdots$

# Sollya
## ���掼���鎮�墰������Ꭾ�⎽������⎦�⎧�⎢ https://www.sollya.org/
- ����ꯎ��Ꭾ�厬꾾墰�㎭�㎼�㎩�㎳�������������������瀎����������墨������
## 쮎������Ꭾ�鎩�����
```sh
>guessdegree(2^x,[-0.5,0.5],1b-23);
[5;5]
```
## ��뎠�玼�ꎿ�掼��
```sh
>fpminimax(2^x,[|1,2,3,4,5|],[|D...|],[-0.5,0.5],1);
1 + x * (0.69314697759916432673321651236619800329208374023437
   + x * (0.24022242085378028852993281816452508792281150817871
   + x * (5.5507337432541360711102385039339424110949039459229e-2
   + x * (9.6715126395259202324306002651610469911247491836548e-3
   + x * 1.326472719636653634089906717008489067666232585907e-3))))
```
# �⎢�㎫�⎴�㎪�⎺�㎠�Ꭾ�Ꮎ�Ꭸ��
## exp(x)�Ꭾ���掼���鎮�
- �厥�� : x
- �玺�� : exp(x)
- 輶������ : c[i] : ��뎠�玼�墰����ʲ
1. $y \leftarrow x \times \log_2(e).$
1. $n \leftarrow {\tt round}(x).$  ������Ꭷ ${\tt round}$ �Ꭿ���躪��窧�����������
1. $a \leftarrow x - n.$
1. $w=2^a \leftarrow 1 + a(c[1] + a(c[2] + a(c[3] + a(c[4] + a c[5])))).$
1. $z \leftarrow 2^n.$
1. return $zw$.

# $2^n w$ �Ꭾ�鎮�
## ���玸���Ꭾ����쵕
- float�Ꭾ�������ꎡ���

float�Ꭾ�������ꎡ���|��s|���ʲ�㎨e|轎������㎨f
-|-|-|-
��������ʹ|1|8|23
- $x=(-1)^s \times 2^{e-127} \times (1+f/2^{23}).$
- `(n+127)<<23`�� $2^n$ �Ꭻ걎��������
## AVX-512�Ꭾꢎ����
- n : float���墰��������
- w : float����
- vscalefps(n, w) �Ꭷ $n \leftarrow 2^n \times w$ �Ꭸ�Ꭺ��

# ���躪��窧
## cvtps2dq
- float����ntꦲ�� : AVX-512�Ꭷ躎���妤�㎼�����莶��墩����
- 躎���妤�㎼��(ctrl)
  - round-to-nearest-even : 躎���妤�㎼��墰�����⎩�㎫��
  - round-toward-zero : 0�Ꭻ������������墭躎�����
  - round-down : 0�Ꭻ�Ꭰ���̻���墭躎�����
  - round-up : 0�Ꭻ�Ꭰ���̻���墭躎�����
- ������Ꭿint���墬�Ꭾ�Ꭷfloat���墺�Ꭾꦲ����玿�ꎦ�
## vrndscaleps
- ${\tt ROUND}(x) = 2^{-M} {\tt round}(x \times 2^M, {\tt ctrl}).$
- ������Ꭿfloat���

# vreduceps
## ��墭겾�ʲ�㎨���莱������¿轎�
- ${\tt vreduceps}(x)=x - {\tt ROUND}(x).$

## �㎬�⎤��妵�⎷�Ꭸ�⎹�㎫�㎼�������
��轎�|�㎬�⎤��妵�⎷|�⎹�㎫�㎼�������
-|-|-
vrndscaleps|8|1
vreduceps|4|0.5-1

��墭���������㎨�����墭겾�ʲ�㎨��
-|-
n �� vrndscaleps(x)  | a �� vreduceps(x)
a �� x - n           | n �� x - a
- ��墭겾�ʲ�㎨����쳤�����̻�������Ꭶ�㎽����������������a�����Ꭻ���ꎦ�񎼉

# ��ꎣ�掾�
## �⎳�⎢�㎨��
- �厥�� : v0
- �玺�� : v0 = exp(v0)
- 輶������ : log2_e, expCoeff[5]�Ꭾ�㎬�⎸�⎹�⎿�Ꭻ�������ꎨ�������Ꭶ����
```python
# v0 = exp(v0)
vmulps(v0, v0, self.log2_e)
vreduceps(v1, v0, 0) # a = x - n
vsubps(v0, v0, v1) # n = x - a = round(x)

vmovaps(v2, self.expCoeff[5])
for i in range(4, -1, -1):
  vfmadd213ps(v2, v1, self.expCoeff[i])
vscalefps(v0, v2, v0) # v2 * 2^n
```

# s_xbyak�Ꭻ�����㎫�㎼���夤�㎳�㎭�㎼�㎫
## Python�Ꭻ�������失�㎭���墬迎����̻
- ���
```python
vfmadd231ps zmm0, zmm0, zmm5
vfmadd231ps zmm1, zmm1, zmm5
vfmadd231ps zmm2, zmm2, zmm5
vfmadd231ps zmm3, zmm3, zmm5
```
�Ꭿ
```python
v = [zmm0, zmm1, zmm2, zmm3]
unroll(vfmadd231ps)(v, v, zmm5)
```
�Ꭸ���墡�Ꭸ��墲����ֺ������Ꭸ�����莩

# �㎡�㎢�㎪��骩�Ꭾ�㎫�㎼���夤�㎳�㎭�㎼�㎫
## ���失�㎭�Ꭾ����
```python
vfaddps(zmm0, zmm0, ptr(rax))
vfaddps(zmm1, zmm1, ptr(rax+64))
vfaddps(zmm2, zmm2, ptr(rax+128))
```
�Ꭺ��
```python
v = [zmm0, zmm1, zmm2]
unroll(vfaddps)(v, v, ptr(rax))
```
- �玪�������Ꭻ�⎪���夽�������掻�掸����Ꭶ�Ꮋ�����
- ptr_b���掽���Ꭳ�Ꭶ��墡��夬���夽�����Ꭿ�Ꭴ���墬��墩�Ꮋ�����?

# �⎢�㎳�㎭�㎼�㎫��ꦿ�Ꭾ���
## Python�Ꭾ�玪��꼎��Ꭾ�������簫�������
```python
def Unroll(n, op, *args, addrOffset=None):
  xs = list(args)
  for i in range(n):
    ys = []
    for e in xs:
      if isinstance(e, list): # ���ʲ�������墬��������������簫�������
        ys.append(e[i])
      elif isinstance(e, Address): # ���ʲ��夤��妮�⎹�Ꭺ��
        if addrOffset == None:
          if e.broadcast:
            addrOffset = 0 # broadcast�㎢�㎼��墬��夬���夽����0������Ꭾ��墡��墱꧎��Ꮏ�Ꭷ���
          else:
            addrOffset = SIMD_BYTE # ������Ꭷ�Ꭺ��墪��墱SIMD�Ꭾ�⎵�⎤�⎺�������(addrOffset�Ꭷ������莶�����Ꭿ�Ꭷ����)
        ys.append(e + addrOffset*i)
      else:
        ys.append(e)
    op(*ys)
```

# unroll�Ꭾ�莩�����
## ���妭���妾��������
- �⎢�㎳�㎭�㎼�㎫����ʲN���玼��ʲ�Ꭸ���墨��夬���妫�㎳���p�������玼��ʲ�Ꭻ걎����墨�⎢�㎳�㎭�㎼�㎫����������������ꎿ���
```python
def genUnrollFunc(n):
  """
    return a function takes op and outputs a function that takes *args and outputs n unrolled op
  """
  def fn(op, addrOffset=None):
    def gn(*args):
      Unroll(n, op, *args, addrOffset=addrOffset)
    return gn
  return fn
  ```

```python
# v0 = [zmm0, zmm1, ...], v1 = [zmm4, zmm5, ...], ...
un = genUnrollFunc(n) # �⎢�㎳�㎭�㎼�㎫����ʲ�����������
un(vmulps)(v0, v0, self.log2_e)
un(vreduceps)(v1, v0, 0) # a = x - n
un(vsubps)(v0, v0, v1) # n = x - a = round(x)
```

# �⎢�㎳�㎭�㎼�㎫��������夵�㎼��
## ��ʲ�Ꭾ�㎬�⎸�⎹�⎿
```python
v0=[zmm0, zmm1, zmm2]
un(vmulps)(z0, z0, log2_e)  �� vmulps(zmm0, zmm0, log2_e)
                              vmulps(zmm1, zmm1, log2_e)
                              vmulps(zmm2, zmm2, log2_e)
```
## �㎡�㎢�㎪��骩
```python
v0=[zmm0, zmm1, zmm2]
v1=[zmm2, zmm3, zmm4]
un(vfmadd231ps)(v0, v1, ptr(rax)) �� vfmadd231ps(zmm0, zmm2, ptr(rax))
                                    vfmadd231ps(zmm1, zmm3, ptr(rax+64))
                                    vfmadd231ps(zmm2, zmm4, ptr(rax+128))
```
# exp(x)�Ꭾ���妵�����㎼�⎯
## �⎢�㎳�㎭�㎼�㎫����ʲN���玤����Ꭶ캎�갚

N|1|2|3|4|5|6|7|8
-|-|-|-|-|-|-|-|-
�厨�Ꭶ�㎬�⎸�⎹�⎿|17.91|15.89|14.14|13.85|13.68|13.08|13.03|13.78
���ʲ����tr_b�Ꭷ𬎭��|18.06|16.21|14.82|14.37|14.54|14.61|14.66|16.19

- �厨�Ꭶ�㎬�⎸�⎹�⎿�Ꭻ���玠����墱N=7������墥��
  - ������夤�㎳�㎭�㎼�㎫������Ꭸ��轎��⎭�㎣��夹�㎥����ة�������墨�������
- 躀�㎨�Ꭾ���ʲ���妣�㎢�㎪�Ꭻ���墡ꢎ���墱N=4������墥��
- �厨��墰�⎳�㎼��墱[fmath](https://github.com/herumi/fmath)�Ꭾ[gen_fmath.py](https://github.com/herumi/fmath/blob/master/gen_fmath.py)

# log(x)�Ꭾ���掼���鎮�
## �⎢�㎫�⎴�㎪�⎺�㎠�Ꭾ������
- $x$ �� $2^n a$($n$ �Ꭿ�������� $1 \le a < 2$)�Ꭾ꿎��Ꭻ��ꎧ�������
- $\log(x) = n \times \log(2) + \log(a).$
- $1 \le a < 2$ �Ꭻ걎������ $\log(a)$ ���莱��������Ꭸ�Ꭻ겤玿�������
## �㎭�㎼�㎩�㎳�����
- $\log(1+x) = x - x^2/2 + x^3/3 - x^4/4 + \cdots.$
- $x \sim 1$ �Ꭷ�Ꭿ���ڡ������

# ���ڡ������������������Ꭻ�ִ���鶯����
## ꦎ�躶���Ꭰ��
- $b$ �� $1/a$ �Ꭾ���掼�������Ꭸ�����
- $c = ab-1$ �Ꭸ���墪$b\sim 1/a$ �Ꭺ�Ꭾ�Ꭷ $c$ �Ꭿ0�Ꭻ�����
- $a=(1+c)/b$ �Ꭸꦲ玽�����倢环�������������墪
- $\log(a) = \log(1+c) - \log(b).$
## ���� $\log(b)$ ���ꎨ�鎮�墩�����Ꭺ��
- $a$ ����������0�Ꭻ����� $c$ �Ꭻ�Ꭴ��墨 $\log(1+c)$ ���莱�����墲����
## $\log(b)$ �Ꭾ�鎮�
- $a$ �Ꭿ[1, 2)�Ꭾ�ִ�Ꭺ�Ꭾ�Ꭷ躴掽�9�������(��+���ʲ�㎨)�Ꭿ������������
- 轎������㎨�Ꭾ躴掽��������Ꭷ��妾���妭�㎫��失�⎢�����������墲 $b$ ��莱�����

# ��妾���妭�㎫��失�⎢����
## $a$ �Ꭾ轎������㎨�Ꭾ躴掽������������莩�������
![width:700px](images/log-table.png)
- $a$ �Ꭾ躶掽� $23-L$ ������������夻�⎯���墨���掼������ $a'$ ���玾���
- $d=m>>(23-L)$ ���夦�㎳�����⎯�⎹�Ꭸ�����
- tbl1�Ꭻ�Ꭿ $1/a'$, tbl2�Ꭻ�Ꭿ $\log(1/a')$ �Ꭾ�������窧��墨����

# ��妾���妭�Ꭾ����
## Python�⎳�㎼��
```python
self.logTbl1 = []
self.logTbl2 = []
self.L = 4
LN = 1 << self.L
for i in range(LN):
  u = (127 << 23) | ((i*2+1) << (23 - self.L - 1))
  v = 1 / uint2float(u)
  v = uint2float(float2uint(v)) # (X) 32�������墰float�����Ꭻ꾎��莶ꦲ��
  self.logTbl1.append(v)
  self.logTbl2.append(math.log(v))
```
- uint2float�Ꭿuint32_t�Ꭾ�����Ꭻ걎���������loat�������玾�����������
- float2uint�Ꭿfloat�����Ꭻ걎���������int32_t�Ꭾ�������玾�����������
## (X)�Ꭾ���µ
- Python�Ꭾfloat�ᎯC�Ꭻ�������ouble�Ꭺ�Ꭾ�Ꭷ�����Ꭾ�Ꮎ�Ꮎ迎���墪�⎤�㎳�����⎯�⎹��񎼰现������������

# AVX-512�Ꭻ���������㎼���妭�㎫��失�⎢����
## vgather��轎�
```python
vgatherdd(out|k, ptr(rax + idx*4)) # out[i] = *(rax + idx[i])
```
- ��ʲ�Ꭾ�⎤�㎳�����⎯�⎹��ôIMD�㎬�⎸�⎹�⎿�Ꭻ�厥��墨1��轎��Ꭷ��ʲ�Ꭾ��妾���妭��骩���ꎡ���
- �����莩�Ꭰ������񎼰妮�⎤��妵�⎷20轎�躊+�㎡�㎢�㎪�⎢�⎯�⎻�⎹���
## vpermps
```python
vpermps(x, y, z) # x[i] = y[z[i]]
```
- float��16(=1<<L)��窧�⵻MM�㎬�⎸�⎹�⎿������㎼���妭�Ꭸ���墨�莩�����妮�⎤��妵�⎷3���
## vpermi2ps
- 512������㯻MM�㎬�⎸�⎹�⎿2������妾���妭�Ꭸ���墨�莩�����妮�⎤��妵�⎷3���
- 轴����Ꭿ�������墬�Ꭳ���墰�Ꭷ���掽����墬��

# 轎������㎨�Ꭸ���ʲ�㎨�Ꭾ�玦���
## ����ڧ�Ꭾ����쵕
- float�Ꭾ�������ꎡ�������Ȫ��墨轎������㎨�Ꭸ���ʲ�㎨�������箼��
- e = (x >> 23) - 127, m = x & mask(23)
## AVX-512�Ꭾ����쵕
- vgetexpss # float�Ꭾ���ʲ�㎨�������箼��
- vgetmantss # float�Ꭾ轎������㎨�������箼��
- �������莼�鎮�����ꎨ�墬���ʲ���掿�����������ꎦ����Ꭺ��뎫�뀟

# log(x)�Ꭾ��ꎣ�掾�
## �㎭�㎼�㎩�㎳���������������玦���
- �厥�� : v0
- �玺�� : v1
- 躀���玤��ʲ : v2, v3
- ���ʲ : L = 4, tbl1, tbl2, one=1.0
```python
un(vgetexpps)(v1, v0) # n
un(vgetmantps)(v0, v0, 0) # a
un(vpsrad)(v2, v0, 23 - self.L) # d = v0 >> (23-L)
un(vpermps)(v3, v2, self.tbl1) # b = tbl1[d] = 1/a
un(vfmsub213ps)(v0, v3, self.one) # c = a * b - 1
un(vpermps)(v3, v2, self.tbl2) # log_b = tbl2[d] = log(1/a)
un(vfmsub132ps)(v1, v3, ptr_b(rip+self.LOG2)) # z = n * log2 - log_b
```
- ���������v0�Ꭻ걎����墨 log(v0) ���妯�㎼�㎩�㎳������Ꭷ쳤����

# log(1+x)�Ꭾ�㎭�㎼�㎩�㎳�����
## 4쮎��Ꭷ���墣��墥����ʲ꾾墰ꢎ����
- $\log(1+x)=x-x^2/2+x^3/3-x^4/4.$
## 4쮎��Ꭷ���墣��墥�Ꭶ����ʲ���ꎪ���������墡ꢎ����
- $\log(1+x)=x-0.49999999 x^2 + 0.33339557 x^3 -0.25008487 x^4.$
```c
// ����걎�𬎤깎�
float relErr(float x, float y) { return x == 0 ? 0 : std::fabs(x - y) / std::fabs(x); }
```
- �쎺��ż1-1/32,1+1/32]�Ꭷx��1e-6���墦ꦲ�ֶ���墡�Ꭸ��墰std::log�Ꭸ�Ꭾ𬎤깎�

꾽�����꾽���莭��
-|-|-
���ꦎ�𬎤깎�| 3.54e-7| 2.44e-7
껎����ꎪ��깎�|4.91e-8|3.97e-8

# x~1�Ꭾ�Ꭸ��墰�鎮��̻쵕
## �꼎��ꎣ��
- $\log(1)=0$ �Ꭺ�Ꭾ�Ꭷ $x\sim 1$ �Ꭾ�Ꭸ��墱������㎭�㎼�㎩�㎳��������簫�����墡����������
- ��妾���妭��骩�Ꭾ�玦�����厥��墪�꼎�������Ꭱ��
- �쎺������Ꭷx��1e-6���墦�������墬����std::log(x)�Ꭸ�Ꭾ����걎�𬎤깎����莱�����

�쎺����[0.99,1.01]|[2,3]
-|-|-
���ꦎ�𬎤깎�|2.80e-2|1.19e-7
껎����ꎪ��깎�|2.61e-5|2.38e-8
- �쎺��ż0.99,1.01]�Ꭿ�쎺��ż2,3]�Ꭻ��墻�Ꭶ�꼎��ꎣ�����ꦎ�����

# ��玲������ꎦ�
## 쨤玿�����墬�⎳�㎼��
```python
if math.abs(x-1) < B: # B�Ꭿ�Ꭹ��墬ꤦ�������
  return log(1+x)�Ꭾ�㎭�㎼�㎩�㎳�����
else:
  return ��妾���妭�㎫��失�⎢�����Ꭾ���妯�㎼�㎩�㎳�����
```
## SIMD�Ꭷ�Ꭾ��玲�
- �鎴�����墪�Ꭻ��玲�����墰�Ꭿ��������
- 躎������Ꭾ�鎮������墨������鎵����������⎹�⎯�Ꭷ�Ꮈ�������
```python
v1 = compute_if_true(x)
v2 = compute_if_false(x)
cond = math.abs(x-1) < B # x~1�Ꭾ�莤갚
mask = 0xffffffff if cond else 0
return (v1 & mask) | (v2 & ~mask)
```

# AVX-512�Ꭻ��������괐
## 64�������墰���夻�⎯�㎬�⎸�⎹�⎿k1, ..., k7
- i��������ְ��
  - 1�Ꭺ�ⱻMM�㎬�⎸�⎹�⎿�Ꭾi���������Ꭾ��掽������紻�Ꭻ�����
  - 0�Ꭺ��瀎����玤��ֶ���墬��
## faddps(x|k, y, z)

y|$y_0$|$y_1$|$y_2$|...|$y_{15}$
-|-|-|-|-|-
z|$z_0$|$z_1$|$z_2$|...|$z_{15}$
k|$k_0=1$|$k_1=0$|$k_2=0$|...|$k_{15}=1$
x|$y_0+z_0$|$x_1$|$x_2$|...|$y_{15}+z_{15}$

# ��玲�夵�㎼��墰�쎿�厥迺鎽��
## �鎮�墰췢��
1. �厥�� : v0 = x
1. ��妾���妭�㎫��失�⎢�����玦��玾�
1. v0 �� c = ab - 1
1. v1 �� n * log2 - log_b
1. ������厥��������墩 $|x-1|<B$ �Ꭺ�� v0 �� x-1, v1 �� 0 �Ꭸ�����
1. v2 �� log(1+v0) # �㎭�㎼�㎩�㎳�����
1. v0 �� v2 + v1 # �玺�� log(x)

# �쎿�厥������⎳�㎼��
## |x-1|< B ���鎢�������
- keepX �� v0
```python
un(vsubps)(v2, keepX, self.one) # x-1
un(vandps)(v3, v2, ptr_b(rip+self.C_0x7fffffff)) # |x-1|
un(vcmpltps)(vk, v3, ptr_b(rip+self.BOUNDARY))
un(vmovaps)(zipOr(v0, vk), v2) # c = v0 = x-1
un(vxorps)(zipOr(v1, vk), v1, v1) # z = 0
```
- float�Ꭾ�걎������Ꭿ���躴掽��������������Ꭸ���񎼈0x7fffffff�Ꭸand���
- vcmpltps�Ꭷ��ꎼ����Ꭶ���夻�⎯�㎬�⎸�⎹�⎿vk�Ꭻ��������莠��
- zipOr�Ꭿv0�Ꭸvk�Ꭾ�鎴������r��������妭���妾��������
```python
def zipOr(v, k):
  r = []
  for i in range(len(v)):
    r.append(v[i]|k[i])
  return r
```

# �쎿�厥�����
## �쎿�厥��墪�쎿�厥���墩����걎�𬎤깎����鎢�������

�쎺����[0.99,1.01]|[2,3]
-|-|-
���ꦎ�𬎤깎�|2.80e-2 �� 1.19e-7 |1.19e-7
껎����ꎪ��깎�|2.61e-5 �� 3.02e-8 |2.38e-8
- [0.99,1.01]�Ꭾ�꼎����Ȼ�������墡

## ���玺��
-|�쎿�厥����쎿�厥���
-|-|-
clk|14.96|19.13
- �⎢�㎳�㎭�㎼�㎫����ʲn = 4�Ꭾ�Ꭸ��鎴�4clk�����Ꭺ�Ꭳ��
- ����걎�𬎤깎��Ꭷ�Ꭿ�Ꭺ��鎵��걎�𬎤깎��Ꭷ�����Ꭺ�Ꭸ��񎼯�og�Ꭾ��墬�Ꭹ���墱��莭�����墬��뢺������������

# �㎡�㎢�㎪�⎢�⎯�⎻�⎹�Ꭻ���������⎹�⎯�㎬�⎸�⎹�⎿
## �㎫�㎼������������32�Ꭾ���ʲ�Ꭷ�Ꭺ��玠�����
- ���夻�⎯�㎬�⎸�⎹�⎿���掽���Ꭳ�Ꭶ�����Ꭺ�⎢�⎯�⎻�⎹��������墬��
![width:800px](images/20230616-121218.png)
## ���夻�⎯�㎬�⎸�⎹�⎿���掽����墬��玠����墱ꤦ��������������Ꭺ������墭쵎���

# �⎼�㎭������㎩�⎰
## $k_i$�Ꭻ걎��������妮�⎸�⎹�⎿��0�⎯�㎪�⎢�����
- vmovups(x|k|T_z,ptr(rax))
  - `T_z` �Ꭿxbyak�Ꭾ𣎨�, MASM/GAS�Ꭷ�Ꭿ`{z}`
- ���妾�⎸��������夫�㎫��񎼲墱�㎬�⎸�⎹�⎿�Ꭾ���玭��̤����������������墰�Ꭷ�����Ꭺ��羱�㎽����
- �⎼�㎭������㎩�⎰���墦������Ꭸ��ꎡ����Ꭾx�Ꭾ�����Ꭻ���玭����Ꭺ��墰�Ꭷ����墭�玦��墩����
## vmovups(x|k|T_z, ptr(m))
x|$x_0$|$x_1$|$x_2$|...|$x_{15}$
-|-|-|-|-|-
m|$m_0$|$m_1$|$m_2$|...|$m_{15}$
k|$k_0=1$|$k_1=0$|$k_2=0$|...|$k_{15}=1$
x|$m_0$|$0$|$0$|...|$m_{15}$


# �㎫�㎼���箨��墰쨤�ʧ(1/2)
## �㎡�⎤�㎳�Ꭸ������玦��墭��粴
- �㎡�⎤�㎳�玦��� : 16N�����Ꭿ�⎢�㎳�㎭�㎼�㎫����ʲ������Ꭴ�鎴�����箨������
```python
  mov(rcx, n) # n �Ꭿ�����Ꭾ�鎴������
  jmp(check1L)
  align(32)
  L(lpUnrollL)
  un(vmovups)(v0, ptr(src)) # 16N�����Ꭾfloat���ꎪ���Ꮏ������
  add(src, 64*unrollN)      # 64N���夦����src�⎢��妮�⎹���뀎�����
  func(unrollN, args)       # �������������������Ꮃ�玺��
  un(vmovups)(ptr(dst), v0) # 16N��鎵��������ֺ��ꎾ����
  add(dst, 64*unrollN)      # 64N���夦����dst�⎢��妮�⎹���뀎�����
  sub(n, 16*unrollN)        # 16N�⎫�⎦�㎳�⎿���莸�����
  L(check1L)
  cmp(n, 16*unrollN)
  jae(lpUnrollL)             # n >= 16*N�Ꭾ���妭�㎼��
```
- ��覵�Ꭿsrc��64���夦��夤�㎩�⎤�㎡�㎳������箨�����厥��������movups�Ꭷ�������������墫���
- 쵎��� : ���夻�⎯�㎬�⎸�⎹�⎿���掽����墪�����Ꭺ��墰�Ꭷ躺ꎦ�墬�Ꭸ��墱迎���墬��

# �㎫�㎼���箨��墰쨤�ʧ(2/2)
## ������玦���
- 0 < ecx < 16
```python
  and_(ecx, 15)
  jz(exitL)
  mov(eax, 1)    # eax = 1
  shl(eax, cl)   # eax = 1 << n
  sub(eax, 1)    # eax = (1 << n) - 1
  kmovd(k1, eax) # k1 = eax
  vmovups(zm0|k1|T_z, ptr(src))
  func(1, args)              # �������������������Ꮃ�玺��
  vmovups(ptr(dst)|k1, zm0)
  L(exitL)
```
- `(1<<n)-1`�Ꭷn��墰1��鎫�墦����������⎹�⎯���掽���
- `kmovd(k1, eax)`�Ꭷ���夻�⎯�㎬�⎸�⎹�⎿���ꎨ��갚
- `zmm0|k1|T_z`���掽���Ꭳ�Ꭶ�㎡�㎢�㎪�Ꭾ𬎭�Ꮏ�������������

# �Ꮎ�Ꭸ��
## AVX-512
- �㎬�⎸�⎹�⎿������玤����Ꭾ�Ꭷ�㎫�㎼���夤�㎳�㎭�㎼�㎫������ꎹ
- ���妯�㎼��夯�㎣�⎹���¿轎��Ꭾ轎������Ꭻ���妯�㎼��夯�㎣�⎹�����㎩�⎰�Ꭷ�㎡�㎢�㎪�⎢�⎯�⎻�⎹���莸�����
- ���夻�⎯�㎬�⎸�⎹�⎿���掽���Ꭳ�Ꭶ��玲����Ꮏ�����
- AVX-512겤�Ȫ��轎����簫�������
  - ���������㎨��现��ʲ�㎨�������箼�� : vrndscaleps, vreduceps
  - ���ʲ�㎨��掻�������㎨�������箼�� : vgetexpss, vgetmantss
  - 겾�������㎼���妭�㎫��失�⎢���� : vpermps, vpermi2ps�Ꭺ�Ꭹ
