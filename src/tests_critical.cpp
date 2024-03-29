#include "board.hpp"

#include "xoshiro256ss.h"
#include <random>



template<size_t W, size_t H>
size_t countUntilCritical() {
    xoshiro256ss g;
	State state = defaultState<W,H>;
	size_t count = 0;
	while (true) {
		state.place<0>(1ULL << (2 * std::uniform_int_distribution<int>(0, W*H-1)(g)));
		++count;

		// for (size_t i = 0; i < W*H; ++i) {
		// 	auto newState =
		// }
	}
}