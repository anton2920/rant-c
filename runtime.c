#include "u.h"
#include "builtin.h"
#include "runtime.h"

#include "assert.h"
#include "syscall.h"

uint64 maxAlloc = 1 << 31;

uint64 CStringLength(char *);

/* NOTE(anton2920): https://opensource.apple.com/source/xnu/xnu-2050.9.2/libsyscall/wrappers/memcpy.c. */
/*
  * sizeof(word) MUST BE A POWER OF TWO
  * SO THAT wmask BELOW IS ALL ONES
  */
typedef int	word;		/* "word" used for optimal copy speed */

void *
memcpy(void *dst0, const void *src0, uint64 length)
{
	char	*dst = dst0;
	const char * src = src0;
	uint64 t;

	if (length == 0 || dst == src)		/* nothing to do */
		goto done;

	/*
	 * Macros: loop-t-times; and loop-t-times, t>0
	 */
#define	wsize	sizeof(word)
#define	wmask	(wsize - 1)
#define	TLOOP(s) if (t) TLOOP1(s)
#define	TLOOP1(s) do { s; } while (--t)

	if ((unsigned long)dst < (unsigned long)src) {
		/*
		 * Copy forward.
		 */
		t = (uintptr)src;	/* only need low bits */
		if ((t | (uintptr)dst) & wmask) {
			/*
			 * Try to align operands.  This cannot be done
			 * unless the low bits match.
			 */
			if ((t ^ (uintptr)dst) & wmask || length < wsize)
				t = length;
			else
				t = wsize - (t & wmask);
			length -= t;
			TLOOP1(*dst++ = *src++);
		}
		/*
		 * Copy whole words, then mop up any trailing bytes.
		 */
		t = length / wsize;
		TLOOP(*(word * )dst = *(word * )src; src += wsize; dst += wsize);
		t = length & wmask;
		TLOOP(*dst++ = *src++);
	} else {
		/*
		 * Copy backwards.  Otherwise essentially the same.
		 * Alignment works as before, except that it takes
		 * (t&wmask) bytes to align, not wsize-(t&wmask).
		 */
		src += length;
		dst += length;
		t = (uintptr)src;
		if ((t | (uintptr)dst) & wmask) {
			if ((t ^ (uintptr)dst) & wmask || length <= wsize)
				t = length;
			else
				t &= wmask;
			length -= t;
			TLOOP1(*--dst = *--src);
		}
		t = length / wsize;
		TLOOP(src -= wsize; dst -= wsize; *(word * )dst = *(word * )src);
		t = length & wmask;
		TLOOP(*--dst = *--src);
	}
done:
	return (dst0);
}


void *
memmove(void *s1, const void *s2, uint64 n)
{
	return memcpy(s1, s2, n);
}


int
memequal(void *_s1, void *_s2, uint64 n)
{
	char	*s1 = _s1, *s2 = _s2;

	for ( ; n--; s1++, s2++) {
		if (*s1 != *s2) {
			return 0;
		}
	}

	return 1;
}


void
panic(char *msg)
{
	Write(2, msg, CStringLength(msg), nil);
	Breakpoint;
}


slice
makeslice(uint64 typSize, uint64 len, uint64 cap)
{
	uint64 mem = typSize * cap;
	slice ret;

	if (mem > maxAlloc) {
		if (typSize * len > maxAlloc) {
			panic("runtime: len out of range");
		}
		panic("runtime: cap out of range");
	}

	ret.base = Allocate(mem);
	ret.len = len;
	ret.cap = cap;

	return ret;
}


uint64
newslicecap(uint64 newLen, uint64 oldCap)
{
	uint64 newcap = oldCap;
	uint64 doublecap = newcap + newcap;
	uint64 threshold = 256;

	if (newLen > doublecap) {
		return newLen;
	}

	if (oldCap < threshold) {
		return doublecap;
	}
	while (1) {
		/* Transition from growing 2x for small slices
		 * to growing 1.25x for large slices. This formula
		 * gives a smooth-ish transition between the two.
		 */
		newcap += (newcap + 3 * threshold) >> 2;

		/* We need to check `newcap >= newLen` and whether `newcap` overflowed.
		 * newLen is guaranteed to be larger than zero, hence
		 * when newcap overflows then `uint(newcap) > uint(newLen)`.
		 * This allows to check for both with the same comparison.
		 */
		if (newcap >= newLen) {
			break;
		}
	}

	/* Set newcap to the requested cap when
	 * the newcap calculation overflowed.
	 */
	if (newcap <= 0) {
		return newLen;
	}
	return newcap;
}


uint64
roundupsize(uint64 size)
{
	int	PageSize = 1 << 12;
	return size + (size & (PageSize - 1));
}


slice
growslice(void *oldPtr, uint64 newLen, uint64 oldCap, uint64 num, uint64 typSize)
{
	uint64 newcap = newslicecap(newLen, oldCap);
	uint64 oldLen = newLen - num;
	uintptr lenmem, capmem;
	void * p;
	slice ret;

	if (newLen < 0) {
		panic("growslice: len out of range");
	}

	lenmem = oldLen * typSize;
	capmem = newcap * typSize;
	capmem = roundupsize(capmem);
	newcap = capmem / typSize;
	capmem = newcap * typSize;

	if (capmem > maxAlloc) {
		panic("growslice: len out of range");
	}

	p = Allocate(capmem);
	memmove(p, oldPtr, lenmem);

	ret.base = p;
	ret.len = newLen;
	ret.cap = newcap;

	return ret;
}


string
slicebytetostring(slice s)
{
	byte	 * memory;
	string ret;

	memory = Allocate(s.len);
	assert(memory != nil);

	ret.base = memory;
	ret.len = s.len;

	copy(Slice(ret), s);
	return ret;
}


