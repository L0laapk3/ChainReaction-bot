#pragma once

#include "board.h"
#include "util.hpp"

#include <array>
#include <iostream>



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
constexpr void inline State<W,H>::invertPlayer() {
	players ^= MASK_PLAYER<W,H> & (board | (board >> 1));
}