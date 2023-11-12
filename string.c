#include "u.h"
#include "builtin.h"

int64
FindChar(string s, char c)
{
	uint64 i;

	for (i = 0; i < s.len; ++i) {
		if (s.base[i] == c) {
			return i;
		}
	}

	return - 1;
}


