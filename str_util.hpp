#pragma once
/*
	string function for SSE4.2
	@NOTE
	all functions in this header will access max 16 bytes beyond the end of input string

	@author herumi
	@note modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <stdlib.h>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

namespace str_util_impl {

const size_t strstrOffset = 0;
const size_t strchrOffset = strstrOffset + 96;
const size_t strlenOffset = strchrOffset + 48;

struct StringCode : Xbyak::CodeGenerator {
	StringCode(char *buf, size_t size)
		: Xbyak::CodeGenerator(size, buf)
	{
		Xbyak::CodeArray::protect(buf, size, true);
		Xbyak::util::Cpu cpu;
		if (!cpu.has(Xbyak::util::Cpu::tSSE42)) {
			fprintf(stderr, "SSE4.2 is not supported\n");
			::exit(1);
		}
		/* this check is adhoc */
		const bool isSandyBridge = cpu.has(Xbyak::util::Cpu::tAVX);

		gen_strstr(isSandyBridge);
		printf("strstr size=%d\n", (int)getSize());

		nextOffset(strchrOffset);
		gen_strchr();
		printf("strchr size=%d\n", (int)(getSize() - strchrOffset));

		nextOffset(strlenOffset);
		gen_strlen();
		printf("strlen size=%d\n", (int)(getSize() - strlenOffset));
	}
private:
	void nextOffset(size_t pos)
	{
		size_t cur = getSize();
		if (cur >= pos) {
			fprintf(stderr, "over %d %d\n", (int)cur, (int)pos);
			::exit(1);
		}
		while (cur < pos) {
			nop();
			cur++;
		}
	}
	// char *strstr(str, key)
	void gen_strstr(bool isSandyBridge)
	{
		inLocalLabel();
		using namespace Xbyak;
#ifdef XBYAK64
	#ifdef XBYAK64_WIN
		const Reg64& key = rdx; // 2nd
		const Reg64& t1 = r11;
		const Reg64& t2 = r9;
		mov(rax, rcx); // 1st:str
	#else
		const Reg64& key = rsi; // 2nd
		const Reg64& t1 = r8;
		const Reg64& t2 = r9;
		mov(rax, rdi); // 1st:str
	#endif
		const Reg64& c = rcx;
		const Reg64& a = rax;
#else
		const Reg32& key = edx;
		const Reg32& t1 = esi;
		const Reg32& t2 = edi;
		const Reg32& c = ecx;
		const Reg32& a = eax;
		const int P_ = 4 * 2;
		sub(esp, P_);
		mov(ptr [esp + 0], esi);
		mov(ptr [esp + 4], edi);
		mov(eax, ptr [esp + P_ + 4]);
		mov(key, ptr [esp + P_ + 8]);
#endif

		/*
			strstr(a, key);
			input key
			use t1, t2, c
		*/
		movdqu(xm0, ptr [key]); // xm0 = *key
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
		mov(t2, key); // save key
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
		add(a, 1);
		jmp(".lp");
	L(".notFound");
		xor(eax, eax);
	L(".found");
#ifdef XBYAK32
		mov(esi, ptr [esp + 0]);
		mov(edi, ptr [esp + 4]);
		add(esp, P_);
#endif
		ret();
		outLocalLabel();
	}
	void gen_strchr()
	{
		inLocalLabel();
		using namespace Xbyak;

#ifdef XBYAK64
	#ifdef XBYAK64_WIN
		const Reg64& p = rcx;
		const Reg64& c1 = rdx;
	#else
		const Reg64& p = rdi;
		const Reg64& c1 = rsi;
	#endif
		const Reg64& c = rcx;
		const Reg64& a = rax;
		and(c1, 0xff);
		movq(xm0, c1);
		mov(a, p);
#else
		const Reg32& a = eax;
		const Reg32& c = ecx;
		movzx(eax, byte [esp + 8]);
		movd(xm0, eax);
		mov(a, ptr [esp + 4]);
#endif
		jmp(".in");
	L("@@");
		add(a, 16);
	L(".in");
		pcmpistri(xm0, ptr [a], 0);
		ja("@b");
		jnc(".notfound");
		add(a, c);
		ret();
	L(".notfound");
		xor(a, a);
		ret();
		outLocalLabel();
	}
	void gen_strlen()
	{
		inLocalLabel();
		using namespace Xbyak;
#if defined(XBYAK64_WIN)
		const Reg64& p = rdx;
		const Reg64& c = rcx;
		const Reg64& a = rax;
		mov(rdx, rcx);
#elif defined(XBYAK64_GCC)
		const Reg64& p = rdi;
		const Reg64& c = rcx;
		const Reg64& a = rax;
#else
		const Reg32& p = edx;
		const Reg32& c = ecx;
		const Reg32& a = eax;
		mov(edx, ptr [esp + 4]);
#endif
		mov(eax, 0xff01);
		movd(xm0, eax);

#if 0
		lea(a, ptr [p - 16]);
#else
		mov(a, p);
		jmp(".in");
#endif
	L("@@");
		add(a, 16);
	L(".in");
		pcmpistri(xm0, ptr [a], 0x14);
		jnz("@b");
		add(a, c);
		sub(a, p);
		ret();
		outLocalLabel();
	}
};

template<int dummy = 0>
struct InstanceIsHere {
	static MIE_ALIGN(4096) char buf[4096];
	static StringCode code;
};

template<int dummy>
StringCode InstanceIsHere<dummy>::code(buf, sizeof(buf));

template<int dummy>
char InstanceIsHere<dummy>::buf[4096];

struct DummyCall {
	DummyCall() { InstanceIsHere<>::code.getCode(); }
};

} // str_util_impl

///////////////////////////////////////////////////////////////////////////////////////
// functions like C
// const version of strstr
inline const char *strstr_sse42(const char *str, const char *key)
{
	return ((const char*(*)(const char*, const char*))(char*)str_util_impl::InstanceIsHere<>::buf)(str, key);
}

// non const version of strstr
inline char *strstr_sse42(char *str, const char *key)
{
	return ((char*(*)(char*, const char*))(char*)str_util_impl::InstanceIsHere<>::buf)(str, key);
}

// const version of strchr
inline const char *strchr_sse42(const char *str, int c)
{
	return ((const char*(*)(const char*, int))((char*)str_util_impl::InstanceIsHere<>::buf + str_util_impl::strchrOffset))(str, c);
}

// non const version of strchr
inline char *strchr_sse42(char *str, int c)
{
	return ((char*(*)(char*, int))((char*)str_util_impl::InstanceIsHere<>::buf + str_util_impl::strchrOffset))(str, c);
}

inline size_t strlen_sse42(const char *str)
{
	return ((size_t(*)(const char*))((char*)str_util_impl::InstanceIsHere<>::buf + str_util_impl::strlenOffset))(str);
}

