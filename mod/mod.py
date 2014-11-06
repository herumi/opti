x=0xf23456782390482094809482424242423333333302948244
y=0xf90482094809482424242423333333302948244293423424
p=0xfffffffffffffffffffffffffffffffeffffffffffffffff

unit = (1 << 64) - 1

def modp(x):
	L = x & ((1 << 192) - 1)
	H = x >> 192
	H0 = H & unit
	H1 = (H >> 64) & unit
	H2 = H >> 128
	t = L + H + (H1 << 128) + (H0 << 64) + H2 + (H2 << 64)
	e = t >> 192
	t = t & ((1 << 192) - 1)
	z = t + e + (e << 64)
	return z


xy = x * y
print hex(xy % p)
print hex(modp(xy))
