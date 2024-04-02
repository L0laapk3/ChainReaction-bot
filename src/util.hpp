#pragma once



#if !defined(NDEBUG)
	#define FUCKING_INLINE __attribute__((noinline))
#else
	#define FUCKING_INLINE __attribute__((always_inline)) inline
#endif



template<typename T>
constexpr inline void optFence(T& v) {
	// Stops the compiler from reordering operations through this variable (and making it worse :( )
    asm inline ("" : "+r"(v));
}