#ifndef JOS_INC_TYPES_H
#define JOS_INC_TYPES_H

#ifndef NULL
#define NULL __null
#endif

#ifndef __cplusplus
// Represents true-or-false values
typedef int bool;
#endif

// Explicitly-sized versions of integer types
typedef __signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

// Pointers and addresses are 32 bits long.
// We use pointer types to represent virtual addresses,
// uintptr_t to represent the numerical values of virtual addresses,
// and physaddr_t to represent physical addresses.
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
typedef uint32_t physaddr_t;

// Page numbers are 32 bits long.
typedef uint32_t ppn_t;

// size_t is used for memory object sizes.
typedef uint32_t size_t;
// ssize_t is a signed version of ssize_t, used in case there might be an
// error return.
typedef int32_t ssize_t;

// off_t is used for file offsets and lengths.
typedef int32_t off_t;


// min and max operations
template <typename T>
T min(T a, T b) {
	return a <= b ? a : b;
}

template <typename T>
T max(T a, T b) {
	return a >= b ? a : b;
}

// Rounding operations (efficient when n is a power of 2)
// Round down to the nearest multiple of n
inline uint32_t round_down(uint32_t a, uint32_t n) {
	return a - a % n;
}

template <typename T>
T *round_down(T *a, uint32_t n) {
	return (T *) round_down((uintptr_t) a, n);
}

template <typename T>
T round_up(T a, uint32_t n) {
	return round_down(a + n - 1, n);
}

// Return the offset of 'member' relative to the beginning of a struct type
#define offsetof(type, member)  ((size_t) (&((type*)0)->member))

#endif /* !JOS_INC_TYPES_H */
