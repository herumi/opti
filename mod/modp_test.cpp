#include <mie/gmp_util.hpp>
#include <iostream>

extern "C" void mie_modNIST_P192(uint64_t *z, const uint64_t *x);

int main()
{
	mpz_class x("0xf23456782390482094809482424242423333333302948244");
	mpz_class y("0xf90482094809482424242423333333302948244293423424");
	mpz_class xy = x * y;
	mpz_class p("0xfffffffffffffffffffffffffffffffeffffffffffffffff");
	std::cout << std::hex;
	std::cout << xy % p << std::endl;

	uint64_t buf[6];
	uint64_t out[3];
	memcpy(buf, mie::Gmp::getBlock(xy), sizeof(buf));
	mie_modNIST_P192(out, buf);
	mpz_class w;
	mie::Gmp::setRaw(w, out, 3);
	std::cout << w << std::endl;
}
