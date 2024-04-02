#pragma once

#include "board.h"



template<size_t W, size_t H>
template<bool player, bool quiescence, typename Callable>
inline void State<W,H>::iterateMoves(Callable&& callback) const {
	if constexpr (quiescence)
		return;
	for (int i = 0; i < W*H; ++i) {
		if (callback(i))
			return;
	}
}