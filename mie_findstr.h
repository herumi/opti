#pragma once
/*
	string function for SSE4.2
	@NOTE
	all functions in this header will access max 16 bytes beyond the end of input string
	see http://www.slideshare.net/herumi/x86opti3

	@author herumi
	@note modified new BSD license
	http://opensource.org/licenses/BSD-3-Clause
*/
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
	find [key, key + keySize) in [begin, end)
	return end if not found
*/
char *mie_findStr(const char *begin, const char *end, const char *key, size_t keySize);

/*
	case insensitive find [key, key + keySize) in [begin, end)
	return end if not found
	@note key must not have capital characters [A-Z]
*/
char *mie_findCaseStr(const char *begin, const char *end, const char *key, size_t keySize);

#ifdef __cplusplus
}
#endif
