#pragma once

#include "board.h"

#include <array>
#include <iostream>
#include <bitset>
#include <type_traits>
#include <cassert>
#include <numeric>




constexpr bool LOG_EXPLOSIONS = false;


#if !defined(NDEBUG)
	#define FUCKING_INLINE __attribute__((noinline))
#else
	#define FUCKING_INLINE __attribute__((always_inline)) inline
#endif




template<size_t W, size_t H>
constexpr board_t<W,H> parseBoard(std::string_view str, int offset) {
	board_t<W,H> result = 0;
	for (size_t i = 0; i < str.size(); ++i) {
		board_t<W,H> c = str[i] - '0';
		if (c < offset)
			c = 0;
		else
			c -= offset;
		result |= c << (W * H * 2 - 2 * (i + 1));
	}
	return result;
}

template<size_t W, size_t H>
constexpr board_t<W,H> MASK_PLAYER = [](){
	board_t<W,H> result = 0;
	for (size_t i = 0; i < W * H; ++i)
		result |= 1ULL << (i * 2);
	return result;
}();



template<size_t W, size_t H>
constexpr board_t<W,H> defaultBoard = [](){
	board_t<W,H> board = 0;
	// board: add 1 to each edge cell
	for (size_t i = 0; i < W; ++i) {
		board += 1ULL << (i * 2);
		board += 1ULL << ((H - 1) * W * 2 + i * 2);
	}
	for (size_t i = 0; i < H; ++i) {
		board += 1ULL << (i * W * 2);
		board += 1ULL << (i * W * 2 + (W - 1) * 2);
	}
	return board;
}();

template<size_t W, size_t H>
constexpr board_t<W,H> defaultPlayers = 0;


template<size_t W, size_t H>
constexpr State<W,H> State<W,H>::parse(std::string_view board, std::string_view players) {
	return State<W,H>{ parseBoard<W,H>(board, 0) + defaultBoard<W,H>, parseBoard<W,H>(players, 1) | defaultPlayers<W,H> };
}

template<size_t W, size_t H>
constexpr auto State<W,H>::operator==(const State<W,H>& other) const {
	board_t<W,H> player_mask = MASK_PLAYER<W,H> & ~(players | (players >> 1) | (other.players | (other.players >> 1)));
	return board == other.board && (players & player_mask) == (other.players & player_mask);
}
template<size_t W, size_t H>
constexpr auto State<W,H>::operator!=(const State<W,H>& other) const {
	return !(*this == other);
}

template<size_t W, size_t H>
struct BoardPrinter {
	explicit BoardPrinter(const board_t<W,H>& v) : value(v) { }
	board_t<W,H> value;
};

template<size_t W, size_t H>
std::ostream& operator<<(std::ostream& os, const BoardPrinter<W,H>& board) {
	os << std::endl;
	for (size_t i = H; i-- > 0; ) {
		for (size_t j = W; j-- > 0; )
			os << (((board.value) >> (i * W * 2 + j * 2)) & 0b11);
		os << std::endl;
	}
	return os;
}
template<size_t W, size_t H>
std::ostream& operator<<(std::ostream& os, const State<W,H>& state) {
	os << BoardPrinter<W,H>{ state.board - defaultBoard<W,H> };
	os << "board / players";
	return os << BoardPrinter<W,H>{ state.players & MASK_PLAYER<W,H> };
}
template<size_t W, size_t H>
constexpr State<W,H> defaultState = State<W,H>{ defaultBoard<W,H>, defaultPlayers<W,H> };



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


template<typename T>
constexpr inline void optFence(T& v) {
	// Stops the compiler from reordering operations through this variable (and making it worse :( )
    asm inline ("" : "+r"(v));
}


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