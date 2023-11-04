#ifndef ASSERT_H
#define ASSERT_H

void _assert_fail(char *expr, char *file, unsigned line);

#define assert(expr) \
	do { \
		if (!(expr)) { \
			_assert_fail(#expr, __FILE__, __LINE__); \
		} \
	} while (0)

#endif /* ASSERT_H */
