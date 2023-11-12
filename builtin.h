#ifndef BUILTIN_H
#define BUILTIN_H

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

typedef struct error *error;
typedef struct slice slice;
typedef struct string string;


struct error {
	string (*Error)(error);
};


struct slice {
	void *base;
	uint64 len;
	uint64 cap;
};

slice Slice(string);
slice UnsafeSlice(void *, uint64);
slice SliceLeft(slice, uint64);
slice SliceRight(slice, uint64);
slice SliceLeftRight(slice, uint64, uint64);


struct string {
	byte	*base;
	uint64 len;
};

string String(slice);
string UnsafeString(byte *, uint64);
string UnsafeCString(char *);
string StringLeft(string, uint64);
string StringRight(string, uint64);
string StringLeftRight(string, uint64, uint64);
#define StringLiteral(s) UnsafeString((byte*)s, sizeof(s)-1)

#define append(vs, v) \
	do { \
		int i, num = 1; \
		if ((vs).len + num > (vs).cap) { \
			(vs) = growslice((vs).base, (vs).len + num, (vs).cap, num, sizeof(v)); \
		} \
		for (i = 0; i < num; ++i) { \
			memcpy((char *)(vs).base + (vs).len*sizeof(v), &(v), sizeof(v)); \
			(vs).len += 1; \
		} \
	} while(0)
uint64 copy(slice dst, slice src);
#define make(typ, len, cap) makeslice(sizeof(typ), len, cap)

slice makeslice(uint64, uint64, uint64);

#endif /* BUILTIN_H */
