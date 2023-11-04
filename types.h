#ifndef TYPES_H
#define TYPES_H

#define nil (void *)0

typedef char	int8;
typedef unsigned char	uint8;
typedef unsigned char	byte;

typedef short	int16;
typedef unsigned short	uint16;

typedef int	int32;
typedef unsigned int	uint32;

/* IMPORTANT(anton2920): this is only true on 64 bit arch. */
typedef long	int64;
typedef unsigned long	uint64;

typedef float	float32;
typedef double	float64;

typedef unsigned long	uintptr;

#endif /* TYPES_H */
