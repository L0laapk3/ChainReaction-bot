#include <array>
#include <iostream>
#include <bitset>
#include <type_traits>
#include <cstdint>
#include <cassert>
#include <numeric>
#include <string_view>




constexpr bool LOG_EXPLOSIONS = false;


template<size_t W, size_t H>
requires(W * H * 2 <= 64 && W * H * 2 > 32)
using board_t = uint64_t;

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
struct State {
	board_t<W,H> board;
	board_t<W,H> players;

	constexpr static State parse(std::string_view board, std::string_view players) {
		return State{ parseBoard<W,H>(board, 0) + defaultBoard<W,H>, parseBoard<W,H>(players, 1) | defaultPlayers<W,H> };
	}

    constexpr auto operator==(const State<W,H>& other) const {
		board_t<W,H> player_mask = MASK_PLAYER<W,H> & ~(players | (players >> 1) | (other.players | (other.players >> 1)));
		return board == other.board && (players & player_mask) == (other.players & player_mask);
	}
	constexpr auto operator!=(const State<W,H>& other) const {
		return !(*this == other);
	}

	template<bool PLAYER>
	constexpr void addBomb(board_t<W,H> add);
};

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



template<size_t W, size_t H>
constexpr inline board_t<W,H> addOne(board_t<W,H>& board, board_t<W,H> add) {
	board_t<W,H> exploding = board & (board >> 1) & add;
	// can overflow
	board += add - (exploding << 2);
	return exploding;
}

template <size_t W, size_t H>
constexpr inline void resetEdges(board_t<W,H>& board, board_t<W,H> exploding) {
	board += (exploding & CELLS_LEFT_RIGHT<W,H>) + (exploding & CELLS_UP_DOWN<W,H>);
}


template<size_t W, size_t H>
template<bool PLAYER>
constexpr inline void State<W,H>::addBomb(board_t<W,H> add) {
	if constexpr (LOG_EXPLOSIONS)
		std::cout << "adding bomb for player " << PLAYER << *this << BoardPrinter<W,H>(add);
	if (PLAYER)
		players = ~players;
	players &= MASK_PLAYER<W,H> & (board | (board >> 1));

	board_t<W,H> exploding = addOne<W,H>(board, add);
	resetEdges<W,H>(board, exploding);
	while (exploding) {
		if constexpr (LOG_EXPLOSIONS)
			std::cout << BoardPrinter<W,H>(board) << BoardPrinter<W,H>(exploding) << std::endl;
		if (!(players &= ~exploding)) {
			if constexpr (LOG_EXPLOSIONS)
				std::cout << "Game win condition" << std::endl;
			break;
		}
		board_t<W,H> oldExploding = exploding;
		exploding  = addOne<W,H>(board, (oldExploding & MASK_LEFT<W,H>) >> 2); // left
		exploding |= addOne<W,H>(board, (oldExploding & MASK_RIGHT<W,H>) << 2); // right
		exploding |= addOne<W,H>(board, (oldExploding << (W * 2)) & ((1ULL << (2 * W * H)) - 1)); // up
		exploding |= addOne<W,H>(board, oldExploding >> (W * 2)); // down
		resetEdges<W,H>(board, exploding);
	}

	if (PLAYER)
		players = ~players;
}