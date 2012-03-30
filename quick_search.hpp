#pragma once
/**
	@file
	@brief quick search algorithm

	@author MITSUNARI Shigeo
*/
#include <algorithm>
#include <string.h>
#include <string>

class QuickSearch {
	static const size_t SIZE = 256;
	size_t len_;
	int tbl_[SIZE];
	std::string str_;
public:
	void init(const char *begin, const char *end)
	{
		if (begin == 0) return;
		len_ = end ? end - begin : strlen(begin);
		str_.assign(begin, len_);
		std::fill(tbl_, tbl_ + SIZE, static_cast<int>(len_ + 1));
		for (size_t i = 0; i < len_; i++) {
			tbl_[static_cast<unsigned char>(begin[i])] = len_ - i;
		}
	}
	explicit QuickSearch(const char *begin = 0, const char *end = 0)
	{
		init(begin, end);
	}
	explicit QuickSearch(const std::string& key)
	{
		init(&key[0], &key[0] + key.size());
	}
	const char *find_org(const char *begin, const char *end) const
	{
		while (begin <= end - len_) {
			if (memcmp(&str_[0], begin, len_) == 0) return begin;
			begin += tbl_[static_cast<unsigned char>(begin[len_])];
		}
		return end;
	}
	const char *find(const char *begin, const char *end) const
	{
		while (begin <= end - len_) {
			for (size_t i = 0; i < len_; i++) {
				if (str_[i] != begin[i]) {
					goto NEXT;
				}
			}
			return begin;
		NEXT:
			begin += tbl_[static_cast<unsigned char>(begin[len_])];
		}
		return end;
	}
};

