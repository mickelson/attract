// utility.h	-- by Thatcher Ulrich <tu@tulrich.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Various little utility functions, macros & typedefs.


#ifndef UTILITY_H
#define UTILITY_H

#include "base/tu_config.h"
#include <assert.h>
#include "base/tu_math.h"
#include "base/tu_types.h"
#include "base/tu_swap.h"
#include <ctype.h>
#include <new>
#include <iostream>


#ifdef _WIN32
#ifdef NDEBUG
// It will help users to send messages about bugs
#undef assert
#define assert(x)		\
	if (!(x))		\
{		\
	std::cerr << "assert(" << #x << "): " << __PRETTY_FUNCTION__ << std::endl << __FILE__ << "(" << __LINE__ << ")" << std::endl; \
	exit(0);		\
}
#endif
#endif // _WIN32


// Compile-time assert.  Thanks to Jon Jagger
// (http://www.jaggersoft.com) for this trick.
#define compiler_assert(x)	switch(0){case 0: case x:;}


//
// new/delete wackiness -- if USE_DL_MALLOC is defined, we're going to
// try to use Doug Lea's malloc as much as possible by overriding the
// default operator new/delete.
//
#ifdef USE_DL_MALLOC

void*	operator new(size_t size);
void	operator delete(void* ptr);
void*	operator new[](size_t size);
void	operator delete[](void* ptr);

#else	// not USE_DL_MALLOC

// If we're not using DL_MALLOC, then *really* don't use it: #define
// away dlmalloc(), dlfree(), etc, back to the platform defaults.
#define dlmalloc	malloc
#define dlfree	free
#define dlrealloc	realloc
#define dlcalloc	calloc
#define dlmemalign	memalign
#define dlvalloc	valloc
#define dlpvalloc	pvalloc
#define dlmalloc_trim	malloc_trim
#define dlmalloc_stats	malloc_stats

#endif	// not USE_DL_MALLOC


#ifndef M_PI
#define M_PI 3.141592654
#endif // M_PI




//
// some misc handy math functions
//

inline int64	i64abs(int64 i) { if (i < 0) return -i; else return i; }
inline int	iabs(int i) { if (i < 0) return -i; else return i; }
inline int	imax(int a, int b) { if (a < b) return b; else return a; }
inline int	imin(int a, int b) { if (a < b) return a; else return b; }
inline float	my_fmin(float a, float b) { if (a < b) return a; else return b; }
inline float	my_fmax(float a, float b) { if (a < b) return b; else return a; }

inline int	iclamp(int i, int min, int max) {
	assert( min <= max );
	return imax(min, imin(i, max));
}

inline float	fclamp(float f, float xmin, float xmax) {
	assert( xmin <= xmax );
	return fmax(xmin, fmin(f, xmax));
}

inline float flerp(float a, float b, float f) { return (b - a) * f + a; }

const float LN_2 = 0.693147180559945f;
inline float	my_log2(float f) { return logf(f) / LN_2; }

inline int	fchop( float f ) { return (int) f; }	// replace w/ inline asm if desired
inline int	frnd(float f) { return fchop(f + 0.5f); }	// replace with inline asm if desired


// Handy macro to quiet compiler warnings about unused parameters/variables.
#define UNUSED(x) (void) (x)


// Compile-time constant size of array.
#define TU_ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))


inline size_t	bernstein_hash(const void* data_in, int size, unsigned int seed = 5381)
// Computes a hash of the given data buffer.
// Hash function suggested by http://www.cs.yorku.ca/~oz/hash.html
// Due to Dan Bernstein.  Allegedly very good on strings.
//
// One problem with this hash function is that e.g. if you take a
// bunch of 32-bit ints and hash them, their hash values will be
// concentrated toward zero, instead of randomly distributed in
// [0,2^32-1], because of shifting up only 5 bits per byte.
{
	const unsigned char*	data = (const unsigned char*) data_in;
	unsigned int	h = seed;
	while (size > 0) {
		size--;
		h = ((h << 5) + h) ^ (unsigned) data[size];
	}

	return h;
}


inline size_t	sdbm_hash(const void* data_in, int size, unsigned int seed = 5381)
// Alternative: "sdbm" hash function, suggested at same web page
// above, http::/www.cs.yorku.ca/~oz/hash.html
//
// This is somewhat slower, but it works way better than the above
// hash function for hashing large numbers of 32-bit ints.
{
	const unsigned char*	data = (const unsigned char*) data_in;
	unsigned int	h = seed;
	while (size > 0) {
		size--;
		h = (h << 16) + (h << 6) - h + (unsigned) data[size];
	}

	return h;
}


inline size_t	bernstein_hash_case_insensitive(const void* data_in, int size, unsigned int seed = 5381)
// Computes a hash of the given data buffer; does tolower() on each
// byte.  Hash function suggested by
// http://www.cs.yorku.ca/~oz/hash.html Due to Dan Bernstein.
// Allegedly very good on strings.
{
	const unsigned char*	data = (const unsigned char*) data_in;
	unsigned int	h = seed;
	while (size > 0) {
		size--;
		h = ((h << 5) + h) ^ (unsigned) tolower(data[size]);
	}

	// Alternative: "sdbm" hash function, suggested at same web page above.
	// h = 0;
	// for bytes { h = (h << 16) + (h << 6) - hash + *p; }

	return h;
}

// Dump the internal statistics from malloc() so we can track memory leaks
void dump_memory_stats(const char *from, int line, const char *label);

// return NaN
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4723)  // The divide by zero is intentional.
#endif   // _MSC_VER
inline double get_nan() { double zero = 0.0; return zero / zero; }
#ifdef _MSC_VER
#pragma warning(pop)
#endif   // _MSC_VER

#endif // UTILITY_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
