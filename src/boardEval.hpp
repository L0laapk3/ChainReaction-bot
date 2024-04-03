#pragma once

#include "board.hpp"
#include <bit>



template<size_t W, size_t H>
constexpr inline Score State<W,H>::evaluate() const {
	return (std::popcount((board | (board >> 1)) & MASK_PLAYER<W,H>) - 2 * std::popcount(players)) * MUL_PIECE_ADVANTAGE;
}