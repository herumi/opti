#include <xbyak/xbyak_util.h>

struct VersionInfo {
	int type;
	int model;
	int family;
	int stepping;
	int extModel;
	int extFamily;
	VersionInfo()
	{
		unsigned int data[4];
		Xbyak::util::Cpu::getCpuid(1, data);
		stepping = data[0] & mask(4);
		model = (data[0] >> 4) & mask(4);
		family = (data[0] >> 8) & mask(4);
		type = (data[0] >> 12) & mask(2);
		extModel = (data[0] >> 16) & mask(4);
		extFamily = (data[0] >> 20) & mask(8);
	}
	unsigned int mask(int n) const
	{
		return (1U << n) - 1;
	}
};
