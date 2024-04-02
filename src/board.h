#pragma once

#include <cstdint>
#include <string_view>
#include "score.h"



template<size_t W, size_t H>
requires(W * H * 2 <= 64 && W * H * 2 > 32)
using board_t = uint64_t;



template<size_t W, size_t H>
struct State {
	board_t<W,H> board;
	board_t<W,H> players;

	constexpr static State parse(std::string_view board, std::string_view players);

    constexpr auto operator==(const State<W,H>& other) const;
	constexpr auto operator!=(const State<W,H>& other) const;

	constexpr board_t<W,H> incrCells(board_t<W,H> add);
	template<bool PLAYER>
	constexpr void place(board_t<W,H> add);

	constexpr void invertPlayer();

	constexpr Score evaluate() const;

	template<bool player, bool quiescence, typename Callable>
	void iterateMoves(Callable&& callback) const;

	constexpr size_t countBombs() const;
	constexpr bool isWon() const;

	constexpr void validate() const;
};