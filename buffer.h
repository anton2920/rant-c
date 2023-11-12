#ifndef BUFFER_H
#define BUFFER_H

typedef struct {
	char	*Buf;
	uint64 Len;
	uint64 Head;
	uint64 Tail;
} CircularBuffer;

CircularBuffer NewCircularBuffer(uint64, error *);

void Consume(CircularBuffer *, uint64);
void Produce(CircularBuffer *, uint64);

uint64 RemainingSpace(CircularBuffer *);
slice RemainingSlice(CircularBuffer *);

void Reset(CircularBuffer *);

uint64 UnconsumedLen(CircularBuffer *);
slice UnconsumedSlice(CircularBuffer *);
string Unconsumedstring(CircularBuffer *);

#endif /* BUFFER_H */
