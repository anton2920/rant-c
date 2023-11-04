#ifndef BUFFER_H
#define BUFFER_H

typedef struct {
	char	*Buf;
	uint64 Len;
	uint64 Head;
	uint64 Tail;
} CircularBuffer;

CircularBuffer NewCircularBuffer(uint64, Error **);

void Consume(CircularBuffer *, int);
void Produce(CircularBuffer *, int);

uint64 RemainingSpace(CircularBuffer *);
Slice RemainingSlice(CircularBuffer *);

void Reset(CircularBuffer *);

uint64 UnconsumedLen(CircularBuffer *);
Slice UnconsumedSlice(CircularBuffer *);
String UnconsumedString(CircularBuffer *);

#endif /* BUFFER_H */
