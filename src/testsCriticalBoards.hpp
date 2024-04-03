#pragma once

#include "board.hpp"

#include "lib/xoshiro256ss.h"
#include <random>



template<size_t W, size_t H>
static auto dist = std::uniform_int_distribution<int>(0, W*H-1);

template<size_t W, size_t H>
std::pair<size_t, State<W,H>> countUntilCritical(xoshiro256ss& g) {
	auto state = defaultState<W,H>;
	size_t count = 1;
	while (true) {
		state.players = MASK_PLAYER<W,H>;
		state.place(1ULL << (2 * dist<W,H>(g)));
		++count;

		for (size_t i = 0; i < W*H; ++i) {
			State<W,H> newState{ state.board, MASK_PLAYER<W,H> };
			newState.place(1ULL << (2 * i));
			if (!newState.players)
				return { count, state };
		}

		if (count == 61)
			std::cout << state << std::endl;
	}
}



template<size_t W, size_t H>
void bruteforce() {
	xoshiro256ss g;
	std::array<U64, 256> counts = { 0 };
	for (size_t i = 0; i < 10000000; ++i) {
		auto [count, state] = countUntilCritical<W,H>(g);
		counts[count]++;
	}

	for (size_t i = 0; i < counts.size(); ++i)
		if (counts[i])
			std::cout << i << ": " << counts[i] << std::endl;
}