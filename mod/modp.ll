
define i128 @mul64x64(i64 %x, i64 %y) {
	%x0 = zext i64 %x to i128
	%y0 = zext i64 %y to i128
	%z = mul i128 %x0, %y0
	ret i128 %z
}

define i64 @extract(i192 %x, i192 %shift) {
	%t0 = lshr i192 %x, %shift
	%t1 = trunc i192 %t0 to i64
	ret i64 %t1
}

define i256 @mul192x64(i192 %x, i64 %y) {
entry:
	%x0 = call i64 @extract(i192 %x, i192 0)
	%x1 = call i64 @extract(i192 %x, i192 64)
	%x2 = call i64 @extract(i192 %x, i192 128)

	%x0y = call i128 @mul64x64(i64 %x0, i64 %y)
	%x1y = call i128 @mul64x64(i64 %x1, i64 %y)
	%x2y = call i128 @mul64x64(i64 %x2, i64 %y)

	%x0y0 = zext i128 %x0y to i256
	%x1y0 = zext i128 %x1y to i256
	%x2y0 = zext i128 %x2y to i256

	%x1y1 = shl i256 %x1y0, 64
	%x2y1 = shl i256 %x2y0, 128

	%t = add i256 %x0y0, %x1y1
	%z = add i256 %t, %x2y1
	ret i256 %z
}

; void mie::mul192x192(uint64_t *z, const uint64_t *x, const uint64_t *y);
define void @_ZN3mie10mul192x192EPmPKmS2_(i384* %pz, i192* %px, i192* %py) {
entry:
	%x = load i192* %px
	%y = load i192* %py
	%y0 = call i64 @extract(i192 %y, i192 0)
	%y1 = call i64 @extract(i192 %y, i192 64)
	%y2 = call i64 @extract(i192 %y, i192 128)

	%xy0 = call i256 @mul192x64(i192 %x, i64 %y0)
	%xy1 = call i256 @mul192x64(i192 %x, i64 %y1)
	%xy2 = call i256 @mul192x64(i192 %x, i64 %y2)

	%xy0_1 = zext i256 %xy0 to i384
	%xy1_1 = zext i256 %xy1 to i384
	%xy2_1 = zext i256 %xy2 to i384

	%xy1_2 = shl i384 %xy1_1, 64
	%xy2_2 = shl i384 %xy2_1, 128

	%t = add i384 %xy0_1, %xy1_2
	%z = add i384 %t, %xy2_2
	store i384 %z, i384* %pz
	ret void
}

; NIST_P192
; 0xfffffffffffffffffffffffffffffffeffffffffffffffff
; 
;       0                1                2
; ffffffffffffffff fffffffffffffffe ffffffffffffffff 
; 
; p = (1 << 192) - (1 << 64) - 1
; (1 << 192) % p = (1 << 64) + 1
; 
; L : 192bit
; Hi: 64bit
; x = [H:L] = [H2:H1:H0:L]
; mod p
;    x = L + H + (H << 64)
;      = L + H + [H1:H0:0] + H2 + (H2 << 64)
;[e:x] = L + H + [H1:H0:H2] + [H2:0] ; 2bit(e) over
;      = x + e + (e << 64)

; void mie::modNIST_P192(uint64_t *z, const uint64_t *x);
define void @_ZN3mie12modNIST_P192EPmPKm(i192* %out, i64* %px) {
entry:
	%pL = bitcast i64* %px to i192*
	%pH64 = getelementptr i64* %px, i64 3
	%pH = bitcast i64* %pH64 to i192*
	%L192 = load i192* %pL
	%H192 = load i192* %pH
	%L = zext i192 %L192 to i256
	%H = zext i192 %H192 to i256
	%t1 = add i256 %L, %H ; %t1 = L + H

	%px3 = getelementptr i64* %px, i64 3
	%px4 = getelementptr i64* %px, i64 4
	%px5 = getelementptr i64* %px, i64 5
	%H0_ = load i64* %px3
	%H1_ = load i64* %px4
	%H2_ = load i64* %px5

	%H0 = zext i64 %H0_ to i256
	%H1 = zext i64 %H1_ to i256
	%H2 = zext i64 %H2_ to i256
	%H0_1 = shl i256 %H0, 64
	%H1_1 = shl i256 %H1, 128
	%H10 = or i256 %H1_1, %H0_1
	%H102 = or i256 %H10, %H2 ; [H1:H0:H2]
	%t2 = add i256 %t1, %H102 ; %t2 = L + H + [H1:H0:H2]

	%H2_1 = shl i256 %H2, 64
	%t3 = add i256 %t2, %H2_1 ; %t3 = L + H + [H1:H0:H2] + [H2:0]

	%e = lshr i256 %t3, 192
	%e64 = shl i256 %e, 64
	%t4 = add i256 %t3, %e
	%t5 = add i256 %t4, %e64 ; t3 + e + (e << 64)

	%t6 = trunc i256 %t5 to i192

	store i192 %t6, i192* %out

	ret void
}
