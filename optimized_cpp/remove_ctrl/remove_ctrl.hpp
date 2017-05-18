#pragma once

// original code
extern "C" size_t removeCtrlOrg(char *dst, const char *src, size_t size);
// search and memcpy
extern "C" size_t removeCtrlSearch(char *dst, const char *src, size_t size);
// use mie_string for search
extern "C" size_t removeCtrlCmpestri(char *dst, const char *src, size_t size);
// search and copy
// reserve size + 16 byte buffer for dst
extern "C" size_t removeCtrlCmpestriCopy(char *dst, const char *src, size_t size);
// use avx
extern "C" size_t removeCtrlAVX(char *dst, const char *src, size_t size);

