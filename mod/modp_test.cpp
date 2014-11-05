#include <mie/gmp_util.hpp>
#include <iostream>

namespace mie {
void modNIST_P192(uint64_t *z, const uint64_t *x);
void mul192x192(uint64_t *z, const uint64_t *x, const uint64_t *y);
}

int main()
{
	mpz_class x("0xf23456782390482094809482424242423333333302948244");
	mpz_class y("0xf90482094809482424242423333333302948244293423424");
	mpz_class xy = x * y;
	mpz_class p("0xfffffffffffffffffffffffffffffffeffffffffffffffff");
	std::cout << std::hex;
	std::cout << xy % p << std::endl;
	uint64_t xBuf[3];
	uint64_t yBuf[3];
	memcpy(xBuf, mie::Gmp::getBlock(x), sizeof(xBuf));
	memcpy(yBuf, mie::Gmp::getBlock(y), sizeof(yBuf));
	uint64_t zBuf[6];
	mie::mul192x192(zBuf, xBuf, yBuf);
	mpz_class z;
	mie::Gmp::setRaw(z, zBuf, 6);
	std::cout << "xy:" << xy << std::endl;
	std::cout << "z :" << z << std::endl;

	uint64_t xyBuf[6];
	uint64_t out[3];
	memcpy(xyBuf, mie::Gmp::getBlock(xy), sizeof(xyBuf));
	mie::modNIST_P192(out, xyBuf);
	mpz_class w;
	mie::Gmp::setRaw(w, out, 3);
	std::cout << w << std::endl;
}
