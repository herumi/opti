declare { i64, i1 } @llvm.uadd.with.overflow.i64(i64, i64)

define { i64, i1 } @add_with_carry(i64 %x, i64 %y, i1 %c) alwaysinline naked
{
  %vc1 = call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 %x, i64 %y)
  %v1 = extractvalue { i64, i1 } %vc1, 0
  %c1 = extractvalue { i64, i1 } %vc1, 1
  %zc = zext i1 %c to i64
  %v2 = add i64 %v1, %zc
  %r1 = insertvalue { i64, i1 } undef, i64 %v2, 0
  %r2 = insertvalue { i64, i1 } %r1, i1 %c1, 1
  ret { i64, i1 } %r2
}
define zeroext i1 @addnLLVM(i64* %pz, i64* %px, i64* %py, i64 %n) {
entry:
  %nZero = icmp eq i64 %n, 0
  br i1 %nZero, label %exit, label %lp

lp:
  %i_p = phi i64 [0, %entry], [%i, %lp]
  %c_p = phi i1 [0, %entry], [%c, %lp]

  %px_i = getelementptr i64* %px, i64 %i_p
  %py_i = getelementptr i64* %py, i64 %i_p
  %pz_i = getelementptr i64* %pz, i64 %i_p
  %x = load i64* %px_i, align 64
  %y = load i64* %py_i, align 64

;  %rc1 = call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 %x, i64 %y)
;  %r1 = extractvalue { i64, i1 } %rc1, 0
;  %c = extractvalue { i64, i1 } %rc1, 1
;  %zc = zext i1 %c_p to i64
;  %r2 = add i64 %r1, %zc

  %rc1 = call { i64, i1 } @add_with_carry(i64 %x, i64 %y, i1 %c_p)
  %r2 = extractvalue { i64, i1 } %rc1, 0
  %c = extractvalue { i64, i1 } %rc1, 1

  store i64 %r2, i64* %pz_i

  %i = add i64 %i_p, 1

  %i_eq_n = icmp eq i64 %i, %n
  br i1 %i_eq_n, label %exit, label %lp

exit:
  %r = phi i1 [0, %entry], [%c, %lp]
  ret i1 %r
}


define void @add128_carry(i64* %pz, i64* %px, i64* %py) {
	%x0 = load i64* %px
	%y0 = load i64* %py
	%vc0 = call { i64, i1 } @add_with_carry(i64 %x0, i64 %y0, i1 0)
	%v0 = extractvalue { i64, i1 } %vc0, 0
	store i64 %v0, i64* %pz
	%c0 = extractvalue { i64, i1 } %vc0, 1
	%px1 = getelementptr i64* %px, i64 1
	%py1 = getelementptr i64* %py, i64 1
	%x1 = load i64* %px1
	%y1 = load i64* %py1
	%vc1 = call { i64, i1 } @add_with_carry(i64 %x1, i64 %y1, i1 %c0)
	%v1 = extractvalue { i64, i1 } %vc1, 0
	%pz1 = getelementptr i64* %pz, i64 1
	store i64 %v1, i64* %pz1
	ret void
}

define void @add128zext(i64* %pz, i64* %px, i64* %py) {
	%x0 = load i64* %px
	%y0 = load i64* %py
	%vc0 = call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 %x0, i64 %y0)
	%v0 = extractvalue { i64, i1 } %vc0, 0
	%c0 = extractvalue { i64, i1 } %vc0, 1
	store i64 %v0, i64* %pz
    %c = zext i1 %c0 to i64
	%px1 = getelementptr i64* %px, i64 1
	%py1 = getelementptr i64* %py, i64 1
	%x1 = load i64* %px1
	%y1 = load i64* %py1
	%z = add i64 %x1, %y1
    %t = add i64 %z, %c
	%pz1 = getelementptr i64* %pz, i64 1
	store i64 %t, i64* %pz1
	ret void
}

define void @add128zext2(i64* %pz, i64* %px, i64* %py) {
	%x0 = load i64* %px
	%y0 = load i64* %py
	%vc0 = call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 %x0, i64 %y0)
	%v0 = extractvalue { i64, i1 } %vc0, 0
	%c0 = extractvalue { i64, i1 } %vc0, 1
	store i64 %v0, i64* %pz
    %c = zext i1 %c0 to i64
	%px1 = getelementptr i64* %px, i64 1
	%py1 = getelementptr i64* %py, i64 1
	%x1 = load i64* %px1
	%y1 = load i64* %py1
	%z = add i64 %x1, %c
    %t = add i64 %z, %y1
	%pz1 = getelementptr i64* %pz, i64 1
	store i64 %t, i64* %pz1
	ret void
}

define void @add128select(i64* %pz, i64* %px, i64* %py) {
	%x0 = load i64* %px
	%y0 = load i64* %py
	%vc0 = call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 %x0, i64 %y0)
	%v0 = extractvalue { i64, i1 } %vc0, 0
	store i64 %v0, i64* %pz
	%c0 = extractvalue { i64, i1 } %vc0, 1
	%px1 = getelementptr i64* %px, i64 1
	%py1 = getelementptr i64* %py, i64 1
	%x1 = load i64* %px1
	%y1 = load i64* %py1
	%z = add i64 %x1, %y1
	%t0 = select i1 %c0, i64 1, i64 0
	%t1 = add i64 %z, %t0
	%pz1 = getelementptr i64* %pz, i64 1
	store i64 %t1, i64* %pz1
	ret void
}

define void @add128jmp(i64* %pz, i64* %px, i64* %py) {
	%x0 = load i64* %px
	%y0 = load i64* %py
	%px1 = getelementptr i64* %px, i64 1
	%py1 = getelementptr i64* %py, i64 1
	%x1 = load i64* %px1
	%y1 = load i64* %py1
	%vc0 = call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 %x0, i64 %y0)
	%v0 = extractvalue { i64, i1 } %vc0, 0
	store i64 %v0, i64* %pz
	%c0 = extractvalue { i64, i1 } %vc0, 1
	%pz1 = getelementptr i64* %pz, i64 1
	br i1 %c0, label %carry, label %nocarry
carry:
	%z1 = add i64 %x1, 1
	%z = add i64 %z1, %y1
	store i64 %z, i64* %pz1
	ret void
nocarry:
	%z2 = add i64 %x1, %y1
	store i64 %z2, i64* %pz1
	ret void
}

define void @add128_i128(i128* %pz, i128* %px, i128* %py) {
	%x = load i128* %px
	%y = load i128* %py
	%z = add i128 %x, %y
	store i128 %z, i128* %pz
	ret void
}

declare { i192, i1 } @llvm.usub.with.overflow.i192(i192, i192)
declare { i128, i1 } @llvm.usub.with.overflow.i128(i128, i128)

define void @addMod(i128* %pz, i128* %px, i128* %py, i128* %pp) {
	%x = load i128* %px
	%y = load i128* %py
	%p = load i128* %pp
	%x1 = zext i128 %x to i192
	%y1 = zext i128 %y to i192
	%p1 = zext i128 %p to i192
	%t = add i192 %x1, %y1
	%vc = call { i192, i1 } @llvm.usub.with.overflow.i192(i192 %t, i192 %p1)
	%v = extractvalue { i192, i1 } %vc, 0
	%c = extractvalue { i192, i1 } %vc, 1
	%z = select i1 %c, i192 %t, i192 %v
	%z1 = trunc i192 %z to i128
	store i128 %z1, i128* %pz
	ret void
}
define void @subMod(i128* %pz, i128* %px, i128* %py, i128* %pp) {
	%x = load i128* %px
	%y = load i128* %py
	%p = load i128* %pp
	%vc = call { i128, i1 } @llvm.usub.with.overflow.i128(i128 %x, i128 %y)
	%v = extractvalue { i128, i1 } %vc, 0
	%c = extractvalue { i128, i1 } %vc, 1
	%a = select i1 %c, i128 %p, i128 0
	%z = add i128 %v, %a
	store i128 %z, i128* %pz
	ret void
}

define void @subMod_jmp(i128* %pz, i128* %px, i128* %py, i128* %pp) {
	%x = load i128* %px
	%y = load i128* %py
	%vc = call { i128, i1 } @llvm.usub.with.overflow.i128(i128 %x, i128 %y)
	%v = extractvalue { i128, i1 } %vc, 0
	%c = extractvalue { i128, i1 } %vc, 1
	br i1 %c, label %carry, label %nocarry
nocarry:
	store i128 %v, i128* %pz
	ret void
carry:
	%p = load i128* %pp
	%z = add i128 %v, %p
	store i128 %z, i128* %pz
	ret void
}

;define void @mul128(i256* %pz, i128* %px, i128* %py) {
;	%x = load i128* %px
;	%y = load i128* %py
;	%x1 = zext i128 %x to i256
;	%y1 = zext i128 %y to i256
;	%z = mul i256 %x1, %y1
;	store i256 %z, i256* %pz
;	ret void
;}
define i128 @mul64x64(i64 %x, i64 %y) {
    %x0 = zext i64 %x to i128
    %y0 = zext i64 %y to i128
    %z = mul i128 %x0, %y0
    ret i128 %z
}
define private i192 @mul128x64(i128 %x, i64 %y)  {
  %x0 = call i64 @extract128(i128 %x, i128 0)
  %x0y = call i128 @mul64x64(i64 %x0, i64 %y)
  %x0y0 = zext i128 %x0y to i192
  %x1 = call i64 @extract128(i128 %x, i128 64)
  %x1y = call i128 @mul64x64(i64 %x1, i64 %y)
  %x1y0 = zext i128 %x1y to i192
  %x1y1 = shl i192 %x1y0, 64
  %t0 = add i192 %x0y0, %x1y1
  ret i192 %t0
}
define void @mie_fp_mul128pre(i64* %pz, i128* %px, i128* %py) {
  %x = load i128* %px
  %y = load i128* %py
  %y0 = call i64 @extract128(i128 %y, i128 0)
  %y1 = call i64 @extract128(i128 %y, i128 64)
  %sum0 = call i192 @mul128x64(i128 %x, i64 %y0)
  %t0 = trunc i192 %sum0 to i64
  store i64 %t0, i64* %pz

  %s0 = lshr i192 %sum0, 64
  %xy1 = call i192 @mul128x64(i128 %x, i64 %y1)
  %sum1 = add i192 %s0, %xy1
  %z1 = getelementptr i64* %pz, i32 1
  %p = bitcast i64* %z1 to i192*
  store i192 %sum1, i192* %p
  ret void
}

define i64 @extract128(i128 %x, i128 %shift) {
	%t0 = lshr i128 %x, %shift
	%t1 = trunc i128 %t0 to i64
	ret i64 %t1
}


define void @mie_fp_sqr(i64* %pz, i128* %px) {
  %x = load i128* %px
  %y = load i128* %px
  %y0 = call i64 @extract128(i128 %y, i128 0)
  %y1 = call i64 @extract128(i128 %y, i128 64)
  %sum0 = call i192 @mul128x64(i128 %x, i64 %y0)
  %t0 = trunc i192 %sum0 to i64
  store i64 %t0, i64* %pz

  %s0 = lshr i192 %sum0, 64
  %xy1 = call i192 @mul128x64(i128 %x, i64 %y1)
  %sum1 = add i192 %s0, %xy1
  %z1 = getelementptr i64* %pz, i32 1
  %p = bitcast i64* %z1 to i192*
  store i192 %sum1, i192* %p
  ret void
}

