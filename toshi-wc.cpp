/*
	compare my vresion and toshi's version by herumi

	wget http://www.gutenberg.org/cache/epub/10/pg10.txt 

	g++ -O3 -march=core2 -msse4 -Wall -Wextra -fno-operator-names word_count_sse42.cpp toshi-wc.cpp -DWITH_TOSHI_WC1 && ./a.out pg10.txt

	// word separator : (c==' ') || (8 < c && c < 14)

optimized intel C version:count=824146, clock=24657.598Kclk
SSE4.2 intrinsic version :count=824146, clock=2495.034Kclk
SSE4.2 Xbyak version0    :count=824146, clock=2434.127Kclk
SSE4.2 Xbyak version1    :count=824146, clock=1848.604Kclk
toshi wc                 :count=824147, clock=1731.043Kclk

	g++ -O3 -march=core2 -msse4 -Wall -Wextra -fno-operator-names word_count_sse42.cpp toshi-wc.cpp -DWITH_TOSHI_WC2 && ./a.out pg10.txt

	// word separator : !(c == '\'') || ('0' <= c && c <= '9') && ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))

optimized intel C version:count=855432, clock=24654.874Kclk
SSE4.2 intrinsic version :count=855432, clock=2468.550Kclk
SSE4.2 Xbyak version0    :count=855432, clock=2391.100Kclk
SSE4.2 Xbyak version1    :count=855432, clock=1884.510Kclk
toshi wc(same as intel)  :count=855432, clock=2804.878Kclk

*/
// SSE2 word count version 5
// using a finite state machine
// by Toshihiro Horie
// last revised August 30, 2011

/*
toshi$ curl http://www.gutenberg.org/cache/epub/10/pg10.txt > ~/bible.txt
 
toshi$ md5 ~/bible.txt
MD5 (/Users/toshi/bible.txt) = 3b83dd48fa7db8df0819108f6d50f64f
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.hpp"

typedef enum {
	ST_WORD = 0,
	ST_SPACE = 256
} StateType;
static const unsigned char wc_table[512] = {
	0,1,1,1,1,2,1,1,1,2,2,2,1,2,1,1,1,2,2,2,2,3,2,2,1,2,2,2,1,2,1,1,1,2,2,2,2,3,2,2,2,3,3,3,2,3,2,2,1,2,2,2,2,3,2,2,1,2,2,2,1,2,1,1,1,2,2,2,2,3,2,2,2,3,3,3,2,3,2,2,2,3,3,3,3,4,3,3,2,3,3,3,2,3,2,2,1,2,2,2,2,3,2,2,2,3,3,3,2,3,2,2,1,2,2,2,2,3,2,2,1,2,2,2,1,2,1,1,0,1,1,1,1,2,1,1,1,2,2,2,1,2,1,1,1,2,2,2,2,3,2,2,1,2,2,2,1,2,1,1,1,2,2,2,2,3,2,2,2,3,3,3,2,3,2,2,1,2,2,2,2,3,2,2,1,2,2,2,1,2,1,1,0,1,1,1,1,2,1,1,1,2,2,2,1,2,1,1,1,2,2,2,2,3,2,2,1,2,2,2,1,2,1,1,0,1,1,1,1,2,1,1,1,2,2,2,1,2,1,1,0,1,1,1,1,2,1,1,0,1,1,1,0,1,0,0,
	1,1,2,1,2,2,2,1,2,2,3,2,2,2,2,1,2,2,3,2,3,3,3,2,2,2,3,2,2,2,2,1,2,2,3,2,3,3,3,2,3,3,4,3,3,3,3,2,2,2,3,2,3,3,3,2,2,2,3,2,2,2,2,1,2,2,3,2,3,3,3,2,3,3,4,3,3,3,3,2,3,3,4,3,4,4,4,3,3,3,4,3,3,3,3,2,2,2,3,2,3,3,3,2,3,3,4,3,3,3,3,2,2,2,3,2,3,3,3,2,2,2,3,2,2,2,2,1,1,1,2,1,2,2,2,1,2,2,3,2,2,2,2,1,2,2,3,2,3,3,3,2,2,2,3,2,2,2,2,1,2,2,3,2,3,3,3,2,3,3,4,3,3,3,3,2,2,2,3,2,3,3,3,2,2,2,3,2,2,2,2,1,1,1,2,1,2,2,2,1,2,2,3,2,2,2,2,1,2,2,3,2,3,3,3,2,2,2,3,2,2,2,2,1,1,1,2,1,2,2,2,1,2,2,3,2,2,2,2,1,1,1,2,1,2,2,2,1,1,1,2,1,1,1,1,0,
};


#define SPACE_CHAR ' '
#define TAB_CHAR ' '
#define NEWLINE_CHAR '\n'
#define RETURN_CHAR '\r'

int wc_toshi(const char *buffer, int n)
{
	__m128i v0;
	__m128i v1 = _mm_set1_epi8(-1); // previous v0
	int i;
	unsigned int mask; // 16 bits
	int wordcount = 0;
	// main loop
#if 1 // wc2.c (use popcnt)
#ifdef WITH_TOSHI_WC1
	__m128i vs = _mm_set1_epi8(SPACE_CHAR); 
	__m128i v8 = _mm_set1_epi8(8);     //  8
	__m128i v14 = _mm_set1_epi8(14);  // 14
#else
	__m128i v_sq = _mm_set1_epi8('\'');
	__m128i vc0 = _mm_set1_epi8('0' - 1);
	__m128i vc9 = _mm_set1_epi8('9' + 1);
	__m128i vca = _mm_set1_epi8('a' - 1);
	__m128i vcz = _mm_set1_epi8('z' + 1);
	__m128i vcA = _mm_set1_epi8('A' - 1);
	__m128i vcZ = _mm_set1_epi8('Z' + 1);
	__m128i vff = _mm_set1_epi8(-1);
#endif

	for (i = 0; i < n; i += 16)
	{
		v0 = _mm_load_si128((__m128i*)&buffer[i]);
		// is it a whitespace? yes->0x80, no->0x00
#ifdef WITH_TOSHI_WC1
		v0 = _mm_or_si128(_mm_cmpeq_epi8(v0, vs),
						  _mm_and_si128(_mm_cmpgt_epi8(v0,v8), _mm_cmplt_epi8(v0,v14)));
#else
#if 1
		__m128i tmp;
		// c=='\'', '0'<=c<='9', 'A'<=c<='Z', 'a'<=c<='z'
		tmp = _mm_or_si128(_mm_cmpeq_epi8(v0, v_sq), _mm_and_si128(_mm_cmpgt_epi8(v0, vc0), _mm_cmplt_epi8(v0, vc9)));
		tmp = _mm_or_si128(tmp, _mm_and_si128(_mm_cmpgt_epi8(v0, vca), _mm_cmplt_epi8(v0, vcz)));
		tmp = _mm_or_si128(tmp, _mm_and_si128(_mm_cmpgt_epi8(v0, vcA), _mm_cmplt_epi8(v0, vcZ)));
		v0 = _mm_xor_si128(tmp, vff);
#else
	__m128i vzero = _mm_set1_epi8(0);
	__m128i v_sq = _mm_set1_epi8('\'');
	__m128i vn0 = _mm_set1_epi8('0');
	__m128i vn9 = _mm_set1_epi8('9');
	__m128i vA = _mm_set1_epi8('A');
	__m128i vZ = _mm_set1_epi8('Z');
	__m128i va = _mm_set1_epi8('a');
	__m128i vz = _mm_set1_epi8('z');
		// 0<c<'\'', '\''<c<'0', '9'<c<'A', 'Z'<c<'a', 'z'<c, c<0
		v0 = _mm_and_si128(_mm_cmpgt_epi8(v0, vzero), _mm_cmplt_epi8(v0, v_sq));
		v0 = _mm_or_si128(v0, _mm_and_si128(_mm_cmpgt_epi8(v0, v_sq), _mm_cmplt_epi8(v0, vn0)));
		v0 = _mm_or_si128(v0, _mm_and_si128(_mm_cmpgt_epi8(v0, vn9), _mm_cmplt_epi8(v0, vA)));
		v0 = _mm_or_si128(v0, _mm_and_si128(_mm_cmpgt_epi8(v0, vZ), _mm_cmplt_epi8(v0, va)));
		v0 = _mm_or_si128(v0, _mm_cmpgt_epi8(v0, vz));
		v0 = _mm_or_si128(v0, _mm_cmplt_epi8(v0, vzero));
#endif

#endif
		// get shifted input
		v1 = _mm_alignr_epi8(v0, v1, 15); // SSSE3
		//printvec("previ",v1);
		// calculate finite difference
		__m128i vr = _mm_xor_si128(v1, v0);
		//printvec("diffs", vr);
		mask = _mm_movemask_epi8(vr);
		//printf("mask = 0x%08x\n", mask);
		mask = _mm_popcnt_u32(mask);
		//printf("wordcount_popcnt = %d\n", mask);
		wordcount += mask;
		v1 = v0;
	}
	wordcount = (wordcount + 1) >> 1;
#else // wc5.c
	unsigned int state = ST_SPACE;
	unsigned int index; // 5 bits

	for (i = 0; i < n; i += 16)
	{
		v0 = _mm_load_si128((__m128i*)&buffer[i]);
		// v0[k] = whitespace ? 0xFF : 0x00
		v0 = _mm_or_si128(_mm_cmpeq_epi8(v0, vs),
							  _mm_and_si128(_mm_cmpgt_epi8(v0,v8), _mm_cmplt_epi8(v0,v14)));
		mask = _mm_movemask_epi8(v0);
		// count words in next 8 bits
		index = ((mask) & 0xFF);
		index = index | state;
		wordcount += wc_table[index];
		state = (index << 1) & 256;
		// count words in next 8 bits
		index = ((mask >> 8));
		index = index | state;
		wordcount += wc_table[index];
		state = (index << 1) & 256;
	} // end for
#endif
	return wordcount;
}

#ifdef STAND_ALONE
int main(int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "wc-test file\n");
		return 1;
	}
	AlignedArray<char> text;
	if (!LoadFile(text, argv[1])) {
		fprintf(stderr, "can't load `%s`\n", argv[1]);
		return 1;
	}
	int count = wc_toshi(&text[0], text.size());
	printf("count=%d\n", count);
}
#endif

