define void @mie_mul192x192(i384* %pz, i192* %px, i64* %py) {
entry:
	%x192 = load i192* %px, align 64
	%py1 = getelementptr i64* %py, i64 1
	%py2 = getelementptr i64* %py, i64 2
	%x = zext i192 %x192 to i384
	%y0_64 = load i64* %py
	%y1_64 = load i64* %py1
	%y2_64 = load i64* %py2
	%y0 = zext i64 %y0_64 to i384
	%y1 = zext i64 %y1_64 to i384
	%y2 = zext i64 %y2_64 to i384
	%xy0 = mul i384 %x, %y0
	%xy1 = mul i384 %x, %y1
	%xy2 = mul i384 %x, %y2
	%xy1s = shl i384 %xy1, 64
	%xy2s = shl i384 %xy2, 128
	%t = add i384 %xy0, %xy1s
	%z = add i384 %t, %xy2s
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

define void @mie_modNIST_P192(i192* %out, i64* %px) {
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
