#pragma once

#include "board.hpp"
#include "util.hpp"

#include <iostream>
#include <type_traits>
#include <cassert>
#include <numeric>



constexpr bool LOG_EXPLOSIONS = false;


template<size_t W, size_t H>
constexpr board_t<W,H> mask_ver(const size_t blank_column) {
	board_t<W,H> result = ~0ULL;
	board_t<W,H> mask_n = 0b11ULL << (blank_column * 2);
	while (mask_n) {
		result &= ~mask_n;
		mask_n <<= W * 2;
	}
	return result;
}

template<size_t W, size_t H>
constexpr board_t<W,H> MASK_LEFT = mask_ver<W,H>(0);
template<size_t W, size_t H>
constexpr board_t<W,H> MASK_RIGHT = mask_ver<W,H>(W - 1);
template<size_t W, size_t H>
constexpr board_t<W,H> CELLS_LEFT_RIGHT = ~MASK_LEFT<W,H> | ~MASK_RIGHT<W,H>;

template<size_t W, size_t H>
constexpr board_t<W,H> CELLS_UP_DOWN = [](){
	board_t<W,H> result = 0;
	board_t<W,H> row = 1ULL;
	row |= row << (W * (H - 1) * 2);
	for (size_t i = 0; i < W; ++i)
		result |= row << (i * 2);
	return result;
}();


template<size_t W, size_t H>
constexpr FUCKING_INLINE board_t<W,H> State<W,H>::incrCells(board_t<W,H> add) {
	players &= ~add;
    optFence(players);
	board_t<W,H> exploding = board & (board >> 1) & add;
	// to avoid overflow, 4 is subtracted from cell when it explodes. 1 or 2 is later added back for edges in resetEdges().
	board += add - (exploding << 2);
	return exploding;
}


template<size_t W, size_t H>
template<bool PLAYER>
constexpr FUCKING_INLINE void State<W,H>::place(board_t<W,H> add) {
	if constexpr (LOG_EXPLOSIONS)
		std::cout << "adding bomb for player " << PLAYER << *this << BoardPrinter<W,H>(add);
	if (PLAYER)
		players ^= MASK_PLAYER<W,H> & (board | (board >> 1));

	board_t<W,H> exploding = incrCells(add);
	while (exploding && players) {
		if constexpr (LOG_EXPLOSIONS)
			std::cout << BoardPrinter<W,H>(board) << BoardPrinter<W,H>(exploding) << std::endl;

		// add back 1 and 2 to exploded edges and corners after incrCells call
		board += (exploding + (exploding << 1)) & defaultBoard<W,H>;

		board_t<W,H> oldExploding = exploding;
        auto left = oldExploding & MASK_LEFT<6,5>;
        optFence(left); // clang needs some nudging to reuse the same mask constant
		exploding  = incrCells(left >> 2); // left
		exploding |= incrCells((oldExploding << 2) & MASK_LEFT<W,H>); // right
		exploding |= incrCells((oldExploding << (W * 2)) & ((1ULL << (2 * W * H)) - 1)); // up
		exploding |= incrCells(oldExploding >> (W * 2)); // down
	}

	if (PLAYER)
		players ^= MASK_PLAYER<W,H> & (board | (board >> 1));
}