#include <mie/gmp_util.hpp>
#include <cybozu/test.hpp>
#include <iostream>

namespace mie {
void modNIST_P192(uint64_t *z, const uint64_t *x);
void mul192x192(uint64_t *z, const uint64_t *x, const uint64_t *y);
}

CYBOZU_TEST_AUTO(mul_and_mod)
{
	mpz_class p("0xfffffffffffffffffffffffffffffffeffffffffffffffff");
	const struct {
		const char *x;
		const char *y;
	} tbl[] = {
		{
			"0xf23456782390482094809482424242423333333302948244",
			"0xf90482094809482424242423333333302948244293423424"
		},
		{
			"0x000000000000000100000000000000020000000000000003",
			"0x000000000000000400000000000000050000000000000006",
		},
	};
	std::cout << std::hex;
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		mpz_class x(tbl[i].x);
		mpz_class y(tbl[i].y);
		uint64_t xBuf[3] = {};
		uint64_t yBuf[3] = {};
		memcpy(xBuf, mie::Gmp::getBlock(x), sizeof(xBuf));
		memcpy(yBuf, mie::Gmp::getBlock(y), sizeof(yBuf));
		uint64_t zBuf[6] = {};
		uint64_t okBuf[6] = {};
		mie::mul192x192(zBuf, xBuf, yBuf);
		mpz_class xy = x * y;
		memcpy(okBuf, mie::Gmp::getBlock(xy), sizeof(okBuf));
		CYBOZU_TEST_EQUAL_ARRAY(okBuf, zBuf, 6);

		uint64_t out[3] = {};
		mie::modNIST_P192(out, okBuf);
		mpz_class w = xy % p;
		uint64_t okOut[3] = {};
		memcpy(okOut, mie::Gmp::getBlock(w),  sizeof(okOut));
		CYBOZU_TEST_EQUAL_ARRAY(okOut, out, 3);
		std::cout << w << std::endl;
		mpz_class t;
		mie::Gmp::setRaw(t, out, 3);
		std::cout << t << std::endl;
	}
}
