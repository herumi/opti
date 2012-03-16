#pragma once
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
/*
	strstr by SSE4.2
	strstr(const char *s1, const char *s2);
	@note
	1. this function will access max 16 bytes beyond the end of string
	2. assume *s2 != '\0'

	@author herumi
	@note modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
struct StrstrCode : Xbyak::CodeGenerator {
	const void *ptr_;
	StrstrCode()
		: ptr_(0)
	{
		gen(false);
		align(16);
		ptr_ = (const void*)getCode();
		gen(true);
	}
	void gen(bool isQs)
	{
		Xbyak::util::Cpu cpu;
		if (!cpu.has(Xbyak::util::Cpu::tSSE42)) {
			fprintf(stderr, "SSE4.2 is not supported\n");
			exit(1);
		}
		const bool isSandyBridge = cpu.has(Xbyak::util::Cpu::tAVX);
		inLocalLabel();
		using namespace Xbyak;
#ifdef XBYAK64
	#ifdef XBYAK64_WIN
		const Reg64& s1 = r10; // 1st
		const Reg64& s2 = rdx; // 2nd
		const Reg64& t1 = r11;
		const Reg64& t2 = r9;
		const Reg64& len = r8; // 3rd
		const Reg64& tbl = r12;
		if (isQs) {
			mov(ptr [rsp + 8], r12);
			mov(tbl, r9); // 4th
		}
		mov(rax, rcx);
	#else
		const Reg64& s1 = rdi; // 1st
		const Reg64& s2 = rsi; // 2nd
		const Reg64& t1 = r8;
		const Reg64& t2 = r9;
		const Reg64& len = rdx; // 3rd
		const Reg64& tbl = r10;
		mov(rax, rdi);
		if (isQs) {
			mov(tbl, rcx); // 4th
		}
	#endif
		const Reg64& c = rcx;
		const Reg64& a = rax;
#else
		const Reg32& s2 = edx;
		const Reg32& t1 = esi;
		const Reg32& t2 = edi;
		const Reg32& c = ecx;
		const Reg32& a = eax;
		const int P_ = 4 * 2;
		sub(esp, P_);
		mov(ptr [esp + 0], esi);
		mov(ptr [esp + 4], edi);
		mov(eax, ptr [esp + P_ + 4]);
		mov(s2, ptr [esp + P_ + 8]);
#endif

		/*
			strstr(a, s2);
			input s2
			use t1, t2, c
		*/
		movdqu(xm0, ptr [s2]); // xm0 = *s2
	L(".lp");
		if (isSandyBridge) {
			pcmpistri(xmm0, ptr [a], 12); // 12(1100b) = [equal ordered:unsigned:byte]
			lea(a, ptr [a + 16]);
			ja(".lp"); // if (CF == 0 and ZF = 0) goto .lp
		} else {
			pcmpistri(xmm0, ptr [a], 12);
			jbe(".headCmp"); //  if (CF == 1 or ZF == 1) goto .headCmp
			add(a, 16);
			jmp(".lp");
	L(".headCmp");
		}
		jnc(".notFound");
		// get position
		if (isSandyBridge) {
			lea(a, ptr [a + c - 16]);
		} else {
			add(a, c);
		}
		mov(t1, a); // save a
		mov(t2, s2); // save s2
	L(".tailCmp");
		movdqu(xm1, ptr [t2]);
		pcmpistri(xmm1, ptr [t1], 12);
		jno(".next"); // if (OF == 0) goto .next
		js(".found"); // if (SF == 1) goto .found
		// rare case
		add(t1, 16);
		add(t2, 16);
		jmp(".tailCmp");
	L(".next");
		if (isQs) {
			mov(Reg32(t1.getIdx()), ptr [a + len]);
			add(a, dword [tbl + t1 * 4]);
		} else {
			add(a, 1);
		}
		jmp(".lp");
	L(".notFound");
		xor(eax, eax);
	L(".found");
#ifdef XBYAK32
		mov(esi, ptr [esp + 0]);
		mov(edi, ptr [esp + 4]);
		add(esp, P_);
#elif defined(XBYAK64_WIN)
		if (isQs) {
			mov(r12, ptr [rsp + 8]);
		}
#endif
		ret();
		outLocalLabel();
	}
} s_strstrCode;

const char* (*strstr_sse42)(const char*,const char*) = (const char* (*)(const char*,const char*))s_strstrCode.getCode();

const char* (*qs_find)(const char*,const char*,size_t,const int*) = (const char* (*)(const char*,const char*,size_t,const int*))s_strstrCode.ptr_;

struct QuickSearch2 {
	static const size_t SIZE = 256;
	size_t len_;
	int tbl_[SIZE];
	std::string str_;
	void init(const char *begin)
	{
		std::fill(tbl_, tbl_ + SIZE, static_cast<int>(len_ + 1));
		for (size_t i = 0; i < len_; i++) {
			tbl_[static_cast<unsigned char>(begin[i])] = len_ - i;
		}
	}
public:
	explicit QuickSearch2(const char *begin, const char *end = 0)
		: len_(end ? end - begin : strlen(begin))
		, str_(begin, len_)
	{
		init(begin);
	}
	explicit QuickSearch2(const std::string& key)
		: len_(key.size())
		, str_(key)
	{
		init(&key[0]);
	}
	const char *find(const char *begin) const
	{
		return qs_find(begin, &str_[0], len_, tbl_);
	}
};
