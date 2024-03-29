#include <array>
#include <iostream>
#include "x86intrin.h"
#include <xmmintrin.h>
#include <bitset>
#include <type_traits>
#include <cstdint>
#include <cassert>
#include <numeric>
#include <string>



typedef uint64_t board_t;

template<size_t width, size_t height>
board_t parseBoard(std::string str, int offset) {
	board_t result = 0;
	for (size_t i = 0; i < str.size(); ++i) {
		board_t c = str[i] - '0';
		if (c < offset)
			c = 0;
		else
			c -= offset;
		result |= c << (width * height * 2 - 2 * (i + 1));
	}
	return result;
}

template<size_t width, size_t height>
constexpr board_t PLAYER_MASK = [](){
	board_t result = 0;
	for (size_t i = 0; i < width * height; ++i)
		result |= 1ULL << (i * 2);
	return result;
}();

template<size_t width, size_t height>
struct State {
	board_t board;
	board_t players;

	static State parse(std::string board, std::string players) {
		return State{ parseBoard<width, height>(board, 0) + defaultState<width, height>.board, parseBoard<width, height>(players, 1) | defaultState<width, height>.players };
	}

    auto operator==(const State<width, height>& other) const {
		board_t player_mask = PLAYER_MASK<width, height> & ~(players | (players >> 1) | (other.players | (other.players >> 1)));
		return board == other.board && (players & player_mask) == (other.players & player_mask);
	}
	auto operator!=(const State<width, height>& other) const {
		return !(*this == other);
	}
};

template<size_t width, size_t height>
constexpr State<width, height> defaultState = [](){
	State<width, height> state{ 0, 0 };
	// board: add 1 to each edge cell
	for (size_t i = 0; i < width; ++i) {
		state.board += 1ULL << (i * 2);
		state.board += 1ULL << ((height - 1) * width * 2 + i * 2);
	}
	for (size_t i = 0; i < height; ++i) {
		state.board += 1ULL << (i * width * 2);
		state.board += 1ULL << (i * width * 2 + (width - 1) * 2);
	}
	// players: alternate the unused bits such that the popcnt is balanced when inverted
	board_t add = 2;
	while (add) {
		state.players |= add;
		add <<= 4;
	}
	assert((sizeof(board_t) * 8 - width * height) % 2 == 0);
	board_t leftover = 1ULL << (width * height * 2);
	while (leftover) {
		state.players |= leftover;
		leftover <<= 4;
	}

	return state;
}();


template<size_t width, size_t height>
constexpr board_t mask_ver(const size_t blank_column) {
	board_t result = ~0ULL;
	board_t mask_n = 0b11ULL << (blank_column * 2);
	while (mask_n) {
		result &= ~mask_n;
		mask_n <<= width * 2;
	}
	return result;
}

template<size_t width, size_t height>
constexpr board_t MASK_LEFT = mask_ver<width, height>(0);
template<size_t width, size_t height>
constexpr board_t MASK_RIGHT = mask_ver<width, height>(width - 1);
template<size_t width, size_t height>
constexpr board_t CELLS_LEFT_RIGHT = ~MASK_LEFT<width, height> | ~MASK_RIGHT<width, height>;

template<size_t width, size_t height>
constexpr board_t CELLS_UP_DOWN = [](){
	board_t result = 0;
	board_t row = 1ULL;
	row |= row << (width * (height - 1) * 2);
	for (size_t i = 0; i < width; ++i)
		result |= row << (i * 2);
	return result;
}();


template<size_t width, size_t height>
inline board_t addOne(board_t& board, board_t& newBoard, board_t add) {
	// can overflow
	board_t exploding = board & (board >> 1) & add;
	newBoard += add;
	return exploding;
}

template <size_t width, size_t height>
inline void removeOverflow(board_t& board, board_t exploding) {
	board = board - (exploding << 2) + (exploding & CELLS_LEFT_RIGHT<width, height>) + (exploding & CELLS_UP_DOWN<width, height>);
}


template<size_t width, size_t height>
inline board_t explode(board_t& board, board_t add) {
	board_t exploding = addOne<width, height>(board, board, add);
	removeOverflow<width, height>(board, exploding);
	board_t exploded = 0;
	while (exploding) {
		exploded |= exploding;
		board_t oldExploding = exploding;
		board_t oldBoard = board;
		exploding  = addOne<width, height>(oldBoard, board, (oldExploding & MASK_LEFT<width, height>) >> 2); // left
		exploding |= addOne<width, height>(oldBoard, board, (oldExploding & MASK_RIGHT<width, height>) << 2); // right
		exploding |= addOne<width, height>(oldBoard, board, (oldExploding << (width * 2)) & ((1ULL << (2 * width * height)) - 1)); // up
		exploding |= addOne<width, height>(oldBoard, board, oldExploding >> (width * 2)); // down
		removeOverflow<width, height>(board, exploding);
	}
	return exploded;
}


template<size_t width, size_t height>
inline State<width, height> addBomb(State<width, height> state, board_t add) {
	// add bomb for player 1. Invert state.players for other player
	board_t exploded = explode<width, height>(state.board, add);
	state.players |= exploded;
	return state;
}



template<typename T>
void assert_equal(T&& a, T& b) {
	if (a != b)
		throw std::runtime_error("assertion failed");
}


int main(int, char**) {
	auto state = defaultState<6,5>;
	assert_equal(State<6,5>::parse("000000000000000000000000000010", "000000000000000000000000000010"), state = addBomb(state, 1ULL << (1 * 2)));
	assert_equal(State<6,5>::parse("000000000000000000000000000020", "000000000000000000000000000010"), state = addBomb(state, 1ULL << (1 * 2)));
	assert_equal(State<6,5>::parse("000000000000000000000010000101", "000000000000000000000000000010"), state = addBomb(state, 1ULL << (1 * 2)));
}