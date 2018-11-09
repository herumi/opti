# 楕円曲線, ペアリング演算ライブラリ[mcl](https://github.com/herumi/mcl)の実装詳解

## 対応曲線
* BN曲線 256bitから462bit(内部的には1024bitとかも可能)
* [BLS12-381](https://z.cash/blog/new-snark-curve/)曲線
    * Zcashなどの暗号通貨で利用されている

## L2準同型暗号
* [ASIACCS2018での発表したものを実装](https://dl.acm.org/citation.cfm?doid=3196494.3196552)
* [紛失通信デモ](https://ppdm.jp/ot)
    * クライアントはJavaScript + WebAssembly
    * サーバはx64最適化 + OpenMP

## 実装
* ペアリング, 楕円曲線, 有限体はC++のtemplateクラスや関数
* 基本演算のC apiを提供
    * vector, string, exception, malloc, freeなどのを使わない
    * メモリ管理は呼び出し側で行う形
    * JavaScript, Go, C#などから利用可能
* 有限体の基本演算(add/sub/mul/sqr/invなど)は以下を切り替え可能
    * 多倍長演算ライブラリ(GMP)
    * (32bit/64bit用)LLVMビットコード
        * x64, x86, ARM, ARM64などLLVMが対応するアーキテクチャに対応
        * WebAssembly出力はllvmが非対応(将来対応するかも/してほしい)
        * C++ DSLによる簡易言語からLLVMビットコード生成
    * pure C++による四則演算ライブラリ
        * これを使うとC++が使えればどこでも動くはず
        * WebAssemblyはここで対応

## ペアリング演算
複雑なので略(別の機会に)

## 楕円曲線
* E/Fp : y^2 = x^3 + bとそのtwist同型 E' → E(詳細略)
* G1 = E(Fp)[r] : Fp上の位数rの巡回群(r個の点)
* G2 : E'(Fp^2)[r]のtwist同型の逆像

## 体演算

* p : 254~381bit素数(x64最適化対象範囲)
* 有限体 : Fp = Z/pZ
* Fp2 = Fp[i] / (i^2 + 1) ; Fpの複素数版
* Fp6 = Fp2[v] / (v^3 - Xi) ; Xi := 1 + i
* Fp12 = Fp6[w] / (w^2 - v)
* GT = {x in Fp12 | x^r = 1} ; Fp12の中のr乗根

## 基礎体のx64最適化

実装は[fp_generator.hpp](https://github.com/herumi/mcl/blob/master/src/fp_generator.hpp)

### 256/382bit整数の表現
* N = 4 or 6
* 64bit整数をUnitと呼ぶことにする(ここだけの表現)
* x, y, z : N個のUnit
* little endian
* 例 x = [x3:x2:x1:x0], xiはUnit. x = x0 + (x1 << 64) + (x2 << 128) + (x3 << 192)

### 加算
* x = [x3:x2:x1:x0], y = [y3:y2:y1:y0]のとき
* 下位Unitから繰り上がりを考慮して足す
* (z0, c) = x0 + y0, cは繰り上がりを表す1bitの情報(carry:以下CF)
* (z1, c) = x1 + y1 + c
* (z2, c) = x2 + y2 + c
* (z3, c) = x3 + y3 + c

#### Xbyakでの実装

```
// pz, px, pyは何がしかのレジスタ
void add(const RegExp& pz, const RegExp& px, const RegExp& py, int n)
{
  for (int i = 0; i < n; i++) {
    mov(rax, ptr[px + i * 8]);
    if (i == 0) {
      add(rax, ptr[py + i * 8]);
    } else {
      adc(rax, ptr[py + i * 8]);
    }
    mov(ptr[pz + i * 8], rax);
  }
}
```

生成コードは

```
mov rax, [px]
add rax, [py]
mov [pz], rax
mov rax, [px + 8]
adc rax, [py + 8]
mov [pz + 8], rax
...
```

#### Pack
* レジスタをまとめて扱うクラス
* Pack a(rax, rcx, r8, r9), b(r10, r11, r12, r13);みたいな

```
void add(const Pack& x, const Pack& y)
{
  for (int i = 0; i < x.size(); i++) {
    if (i == 0) {
      add(x[i], y[i]);
    } else {
      adc(x[i], y[i]);
    }
  }
}
```
生成コードは
```
// Pack a(rax, rcx, r8, r9), b(r10, r11, r12, r13);
// add(a, b);

add r9, r13
adc r8, r12
adc rcx, r11
adc rax, r10
```

### 最後の繰り上がり
* 256bit + 256bitの結果は257bitで1bit溢れる
* それは最後のCFで情報を持っている
* 素数を254bitとか381bitにすると最上位bitに空白ができて最後のCF処理を省ける
    * 若干の安全性を減らして速度向上

### 減算
* add/adcの代わりにsub/sbbを使う

```
// pz, px, pyは何がしかのレジスタ
void sub(const RegExp& pz, const RegExp& px, const RegExp& py, int n)
{
  for (int i = 0; i < n; i++) {
    mov(rax, ptr[px + i * 8]);
    if (i == 0) {
      sub(rax, ptr[py + i * 8]);
    } else {
      sbb(rax, ptr[py + i * 8]);
    }
    mov(ptr[pz + i * 8], rax);
  }
}
```

### Fp::sub
* z = (x - y) mod p
* z = x - yを計算して負になったらz += p
* 負かどうかはCFを見れば分かる

#### ナイーブな実装
z = x - yを計算した直後に分岐


```
// x -= y
  jnc("@f");
  add(x, ptr_p);
L("@@");
```

* 分岐予測が外れたときのペナルティは10数clk
* 暗号の計算ではx, yはほぼ乱数なので50%の確率で外れるはず
* 256bitではadd/adc r, mが4回分で2clk x 4程度
* 分岐した方がコストが高い

### 条件分岐命令cmov

* 分岐条件成立ならmovする / そうでなければ何もしない
* 例 : cmovc rax, rdx ; CFが立っているときのみrax ← rdx


### 256bitのFp::sub

```
// x -= y
// t = Pack(4)
mov rax, 0 // (*)
mov t, rax // 簡略表記(意味は伝わるよね)
cmovc t, [ptr_p] // t = CF ? p : 0
add x, t // x += (CF) ? p : 0
```

* 注意
* (*)でxor rax, raxはできない(CFを壊してしまう)

### cmovより論理命令(and, or, xor)の方が速い

* Agner Fogさんの[Instruction table](https://www.agner.org/optimize/instruction_tables.pdf) いつでも見よう



命令|Operand|μops fused|μops unfused|Latency|1/throughput
----|-------|-----------|-------------|-------|------------
add |r, r   |1          |1            |1      |0.25
and |r, r   |1          |1            |1      |0.25
cmov|r, r   |1          |1            |1      |0.5
adc |r, r   |1          |1            |1      |1
mulx|r, r   |2          |3            |4      |1
adox|r, r   |1          |1            |1      |1

r, mになるとどれも+1程度余計にかかる

上記コードは次のように最適化可能

```
// x -= y
sbb rax, rax // rax = CF ? -1 : 0
mov t, [ptr_p]
and t, rax   // t = CF ? p : 0
```

### 384bit(n = 6)では?

* 実験した限りではsbb + andよりjncの方が速かった
* n = 6ではレジスタ6個+退避用のtで6個でレジスタに対する負担が大きい


### Fp::add
* z = (x + y) mod p
* z = x + yを計算してz >= pならz -= p
* subと違ってaddは足してからpを引かないと大きいかどうか分からない

いくつかの戦略方法

* t = z(退避)してからz -= pしてCFが立てばz = tで復元
* z >= pを確認してからpを引く

一手間増えるためsubよりも重たい
* 上位の計算でaddよりもsubを増やす方が得なことがある
* 例 : x - (y + z)よりx - y - z


### 退避してから引き算して復元

```
// x += y
mov t, x       // xを退避
sub x, [ptr_p] // x -= p
cmovc x, t     // CFが立っていればx = tに戻す
```

* この場合andなどの手法は(多分)使えない / 使った方が遅い
* n = 6でもjmpよりcmovのほうが速かった
    * sub(比較)はどのみち必須なので残りはjmpとcmovの差になるから
* レジスタ数が圧迫されて辛い

### 乗算mulPre(n-Unit x n-Unit→2n-Unit)

* 教科書的乗算
* Karatsuba

### 教科書的乗算

mulUnit(n-Unit x Unit→(n+1)-Unit)をして加算

```
   |x2|x1|x0|
x        | y|
-------------
       |x0*y|
    |x1*y|
 |x2*y|
----------
  ......
```

### 昔のmulUnit

* mul reg(reg * rax→[rdx:rax])

* mul命令はCFやOF(over flag)を破壊する
* x0*y, x1*y, x2*yを全部計算してからadd, adcを使わないといけない
* n-Unit x Unitは2n個のレジスタが必要

### mulx H, L, reg(reg * rdx→[H:L])

* mulxはフラグを変更しない
* mulxしてadd/adcすれば一時レジスタは3個のまま計算できる


```
t0, t1 : Reg64
// input : px[n], rdx = y
// output: c[n+1] = px[n] * rdx
mulx(t0, c[0], ptr[px + 0 * 8]);
for (int i = 0; i < n; i++) {
  mulx(t1, c[i], ptr[px + i * 8]);
  if (i == 1) {
    add(c[i], t0);
  } else {
    adc(c[i], t0);
    t0.swap(t1);
  }
  mov(c[n], 0);
  adc(c[n], t0);
}
```

### adox, adcx
多倍長乗算はmulUnitしてからaddすることになる

```
   |x2|x1|x0|=x
x  |y2|y1|y0|
-------------
     |x * y0|
  |x * y1|
|x * y2|
-------------
       ......
```

* |x * y0|と|x * y1|を足すためには|x * y0|を保持しなければならない
* レジスタ圧(略
* n = 4でぎりぎりうまいこと使い回した(がn = 6は無理)
* adoxとadcxを使うとmulUnitとaddを並行にできる
* adcxはadcと同じ(だがOFを変化させない)
* adoxはCFの代わりにOFを使う

方法

* mulxで作った[H:L]に対してLをadoxでHをadcxで足す
* それぞれのlineはOF, CFでcarryを伝達
* 最後のところでOFの帳尻を合わせる


```
// input  : t[4], x[3], y
// output : t[4] += x[3] * y
|t3|t2|t1|t0|
   |x2|x1|x0|
           y
-------------
      |H0|L0| ; [H0:L0] ← x0 * y
   |H1|L1|    ; [H1:L1] ← x1 * y
|H2|L2|       ; [H2:L2] ← x2 * y


xor rax, rax ; CF, OFクリア
mov rdx, y
mulx H, L, ptr[x0]
adox t0, L
adcx t1, H
mulx H, L, ptr[x1]
adox t1, L
adcx t2, H
...
mulx H, L, ptr[x(n-1)]
adox t(n-1), L
adcx tn, H

mov rdx, 0
adcx tn, rdx
adox tn, rdx

```

* すごくきれいにn-Unit x n-Unit→2n-Unitを実現
* しかしn = 4のときは使わない方が速かった
  * adcx, adoxはaddよりちょっと遅い
  * 全部adcx, adoxを使うよりaddが4回ある方がよい???
* n = 6ではレジスタ退避・復元を考えると使った方がよいだろう(多分:使わないバージョンはめんどいので未実装)
* Core-i7 7700で56clk
    * 乗算6 x 6 = 36回
    * 加算7 + (2 * 8) * 5 = 87回

### Karatsuba法
加算を増やす代わりに乗算を減らす
nを偶数とし, N = (1 << (64 * n/2))として
x = aN + b, y = cN + dについて

* xy = (aN + b)(cN + d) = acN^2 + (ad + bc)N + bd
* ad + bc = (a + b)(c + d) - ac - bd

ac, ad + bc, bdはad, bc, (a + b)(c + d)の3回の乗算で可能

### (a + b)(c + d)
n-Unitのa, b, c, dに対してa + b, c + dは1-bit溢れる

```
a + b = [ε:A] ; ε, η : 1bit, A, B : n-Unit
c + d = [η:B]
(a + b)(c + d) = [εη:εB + ηA:AB]
```
この端数処理が結構めんどい

### 教科書 vs. Karatsuba

* n = 4のとき教科書の勝ち
* n = 6のとき教科書の勝ち

### 平方算
* x = aN + bのときx^2 = (aN + b) = a^2N^2 + 2abN + b^2
* 最初から乗算は3回

### Montgomery乗算
* Fp::mul(z, x, y)はz = (x * y) % p
* pで割るのはとても重たい
    * 楕円曲線暗号では「p = (1 << bitLen) - αでαがとても小さい値」の素数を選んで高速化
    * ペアリングではそこまで都合のよいpを選べない

Montgomery還元 : pから一意に決まる数値Rがある

mont(x) := xR mod p

と定義する

montMul(x, y) := xyR^(-1)

という演算を定義する

* montMul(xR, yR) = (xR)(yR)R^(-1)=xyR
* 「xとyを掛けてmod p」する代わりに[xRとyRのmontMulをするとxyRが得られる
* montMulはmod pよりはコストが低い
* xとxRの変換コストは高いが一度xRの世界に移行すれば最後の出力までxRのままで計算する
* xRはmont(x, R^2) = xR^2 R^(-1) = xRで計算できる

### montMulの中身

```
// montMul(x, y);
// input  : x[n], y[n]
// output : z[n] = montMul(x[n], y[n])
z = x * y[0];
Unit q = z[0] * rp_; // rp_はpから定まるUnitの定数
z += p_ * q;
z >>= 64;
for (size_t i = 1; i < n; i++) {
  z += x * y[i];
  Unit q = z[0] * rp_;
  z += p_ * q;
  z >>= 64;
}
if (z >= p_) {
  z -= p_;
}
return z;
```

### Fp2::mul(x, y)

Fp2の乗算はx = a + bi, y = c + di, i^2 = -1に対してz = xyを複素数的に計算する

```
z = xy = (a + bi)(c + di) = (ac - bd) + (ad + bc)i
ad + bc = (a + b)(c + d) - ac - bd
```

* Karatsubaのときのように乗算3回でよい
* このままではそれほどメリットにはならないが
* montMulを分解する

### montMulをmulPreとred(uction)に分解

* montMulはxとyを掛けながらreductionしている
* 先にxとyを掛けてからreductionしても等価

montMul(x, y) = red(x * y)

```
// z = red(xy)
// input  : xy[2n]
// output : z[n] = red(xy)
for (size_t i = 0; i < n; i++) {
  Unit q = xy[0] * rp_;
  xy += p_ * q;
  xy >>= 64;
}
if (xy >= p_) {
  xy -= p_;
}
return xy;
```

* ただしx * yは2n-Unitなのでxy += p_ * q;のコストはmontMulよりも大きい
    * 繰り上がりの回数が多いため
    * レジスタ数も多い(n = 6では足りない)

### Fp2Dbl(2n-Unit)整数
* xとyをmodをとらずに掛けた値A = xyは2n-Unit整数
* Fp2::mulの計算を観察

* A = ac, B = bd, C = (a + b)(c + d)を2n-Unit整数とする
    * これらの演算はmulPre
    * ここでa, b, c, dが(n * 64 - 1)-bit整数ならa + bとc + dは(n * 64)-bit整数
* 必要な値はac - bd = red(A - B)とad + bc = red(C - A - B)
* こうするとmulPreは3回だがredは2回ですむ

### montSqr(x) = montMul(x, x)
* montMul(x, y)はx == yのときの最適化が難しい
* montSqr(x) = red(mulPre(x, x)) = red(sqrPre(x))
* sqrPre()の最適化はKaratsuba的な可能

### montMul(x, x) vs. red(sqrPre(x))

* n = 4のときmontMulの勝ち
* n = 6のときred(sqrPre(x))の勝ち
