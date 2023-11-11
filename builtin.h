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

uint64 copy(slice dst, slice src);




#endif /* BUILTIN_H */
