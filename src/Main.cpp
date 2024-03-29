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




constexpr bool LOG_EXPLOSIONS = true;


template<size_t W, size_t H>
requires(W * H * 2 <= 64 && W * H * 2 > 32)
using board_t = uint64_t;

template<size_t W, size_t H>
constexpr board_t<W,H> parseBoard(std::string str, int offset) {
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

	constexpr static State parse(std::string board, std::string players) {
		return State{ parseBoard<W,H>(board, 0) + defaultBoard<W,H>, parseBoard<W,H>(players, 1) | defaultPlayers<W,H> };
	}

    constexpr auto operator==(const State<W,H>& other) const {
		board_t<W,H> player_mask = MASK_PLAYER<W,H> & ~(players | (players >> 1) | (other.players | (other.players >> 1)));
		return board == other.board && (players & player_mask) == (other.players & player_mask);
	}
	constexpr auto operator!=(const State<W,H>& other) const {
		return !(*this == other);
	}
};

template<size_t W, size_t H>
struct BoardPrinter {
	explicit BoardPrinter(const board_t<W,H>& v) : value(v) { }
	board_t<W,H> value;
};

template<size_t W, size_t H>
std::ostream& operator<<(std::ostream& os, const BoardPrinter<W,H>& board) {
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
	os << "board / players" << std::endl;
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
inline board_t<W,H> addOne(board_t<W,H>& board, board_t<W,H> add) {
	board_t<W,H> exploding = board & (board >> 1) & add;
	// can overflow
	board += add - (exploding << 2);
	return exploding;
}

template <size_t W, size_t H>
inline void resetEdges(board_t<W,H>& board, board_t<W,H> exploding) {
	board += (exploding & CELLS_LEFT_RIGHT<W,H>) + (exploding & CELLS_UP_DOWN<W,H>);
}


template<bool player, size_t W, size_t H>
inline State<W,H> addBomb(State<W,H> state, board_t<W,H> add) {
	if constexpr (LOG_EXPLOSIONS)
		std::cout << "adding bomb for player " << player << std::endl << state << std::endl << BoardPrinter<W,H>(add) << std::endl;
	if (player)
		state.players = ~state.players;
	state.players &= MASK_PLAYER<W,H> & (state.board | (state.board >> 1));

	board_t<W,H> exploding = addOne<W,H>(state.board, add);
	resetEdges<W,H>(state.board, exploding);
	while (exploding) {
		if constexpr (LOG_EXPLOSIONS)
			std::cout << BoardPrinter<W,H>(state.board) << std::endl << BoardPrinter<W,H>(exploding) << std::endl;
		if (!(state.players &= ~exploding)) {
			std::cout << "Game win condition" << std::endl;
			break;
		}
		board_t<W,H> oldExploding = exploding;
		exploding  = addOne<W,H>(state.board, (oldExploding & MASK_LEFT<W,H>) >> 2); // left
		exploding |= addOne<W,H>(state.board, (oldExploding & MASK_RIGHT<W,H>) << 2); // right
		exploding |= addOne<W,H>(state.board, (oldExploding << (W * 2)) & ((1ULL << (2 * W * H)) - 1)); // up
		exploding |= addOne<W,H>(state.board, oldExploding >> (W * 2)); // down
		resetEdges<W,H>(state.board, exploding);
	}

	if (player)
		state.players = ~state.players;
	return state;
}


template<typename T>
void assert_equal(T&& a, T& b) {
	if (a != b) {
		std::cout << a << std::endl;
		std::cout << b << std::endl;
		throw std::runtime_error("assertion failed");
	}
}

State<6,5> test(State<6,5>& state, board_t<6,5> add) {
    return addBomb<1>(state, add);
}


int main(int, char**) {
	auto state = defaultState<6,5>;
	assert_equal(State<6,5>::parse("000000000000000010000000000000", "000000000000000010000000000000"), state = addBomb<1>(state, 1ULL << (13 * 2)));
	assert_equal(State<6,5>::parse("000001000000000010000000000000", "000000000000000010000000000000"), state = addBomb<0>(state, 1ULL << (24 * 2)));
	assert_equal(State<6,5>::parse("000001000000000011000000000000", "000000000000000011000000000000"), state = addBomb<1>(state, 1ULL << (12 * 2)));
	assert_equal(State<6,5>::parse("000001000000000011001000000000", "000000000000000011000000000000"), state = addBomb<0>(state, 1ULL << (9  * 2)));
	assert_equal(State<6,5>::parse("000001000000001011001000000000", "000000000000001011000000000000"), state = addBomb<1>(state, 1ULL << (15 * 2)));
	assert_equal(State<6,5>::parse("000001000000001011001000000100", "000000000000001011000000000000"), state = addBomb<0>(state, 1ULL << (2  * 2)));
	assert_equal(State<6,5>::parse("000001000000001111001000000100", "000000000000001111000000000000"), state = addBomb<1>(state, 1ULL << (14 * 2)));
	assert_equal(State<6,5>::parse("000001010000001111001000000100", "000000000000001111000000000000"), state = addBomb<0>(state, 1ULL << (22 * 2)));
	assert_equal(State<6,5>::parse("000001010000001111011000000100", "000000000000001111010000000000"), state = addBomb<1>(state, 1ULL << (10 * 2)));
	assert_equal(State<6,5>::parse("000001010000001111011000000110", "000000000000001111010000000000"), state = addBomb<0>(state, 1ULL << (1  * 2)));
	assert_equal(State<6,5>::parse("000001010000002111011000000110", "000000000000001111010000000000"), state = addBomb<1>(state, 1ULL << (15 * 2)));
	assert_equal(State<6,5>::parse("000001010000002111011100000110", "000000000000001111010000000000"), state = addBomb<0>(state, 1ULL << (8  * 2)));
	assert_equal(State<6,5>::parse("000001011000002111011100000110", "000000001000001111010000000000"), state = addBomb<1>(state, 1ULL << (21 * 2)));
	assert_equal(State<6,5>::parse("000011011000002111011100000110", "000000001000001111010000000000"), state = addBomb<0>(state, 1ULL << (25 * 2)));
	assert_equal(State<6,5>::parse("000011111000002111011100000110", "000000101000001111010000000000"), state = addBomb<1>(state, 1ULL << (23 * 2)));
	assert_equal(State<6,5>::parse("000011111100002111011100000110", "000000101000001111010000000000"), state = addBomb<0>(state, 1ULL << (20 * 2)));
	assert_equal(State<6,5>::parse("000011111110002111011100000110", "000000101010001111010000000000"), state = addBomb<1>(state, 1ULL << (19 * 2)));
	assert_equal(State<6,5>::parse("000011111110002111011110000110", "000000101010001111010000000000"), state = addBomb<0>(state, 1ULL << (7  * 2)));
	assert_equal(State<6,5>::parse("000011112110002111011110000110", "000000101010001111010000000000"), state = addBomb<1>(state, 1ULL << (21 * 2)));
	assert_equal(State<6,5>::parse("000011112110002111011110001110", "000000101010001111010000000000"), state = addBomb<0>(state, 1ULL << (3  * 2)));
	assert_equal(State<6,5>::parse("000011112110002211011110001110", "000000101010001111010000000000"), state = addBomb<1>(state, 1ULL << (14 * 2)));
	assert_equal(State<6,5>::parse("001011112110002211011110001110", "000000101010001111010000000000"), state = addBomb<0>(state, 1ULL << (27 * 2)));
	assert_equal(State<6,5>::parse("001011112110003211011110001110", "000000101010001111010000000000"), state = addBomb<1>(state, 1ULL << (15 * 2)));
	assert_equal(State<6,5>::parse("001011122110003211011110001110", "000000101010001111010000000000"), state = addBomb<0>(state, 1ULL << (22 * 2)));
	assert_equal(State<6,5>::parse("001011122110003211111110001110", "000000101010001111110000000000"), state = addBomb<1>(state, 1ULL << (11 * 2)));
	assert_equal(State<6,5>::parse("001011122210003211111110001110", "000000101010001111110000000000"), state = addBomb<0>(state, 1ULL << (20 * 2)));
	assert_equal(State<6,5>::parse("011011122210003211111110001110", "010000101010001111110000000000"), state = addBomb<1>(state, 1ULL << (28 * 2)));
	assert_equal(State<6,5>::parse("011011122310003211111110001110", "010000101010001111110000000000"), state = addBomb<0>(state, 1ULL << (20 * 2)));
	assert_equal(State<6,5>::parse("011011122320003211111110001110", "010000101010001111110000000000"), state = addBomb<1>(state, 1ULL << (19 * 2)));
	assert_equal(State<6,5>::parse("011011122320103211111110001110", "010000101010001111110000000000"), state = addBomb<0>(state, 1ULL << (17 * 2)));
	assert_equal(State<6,5>::parse("011011222320103211111110001110", "010000101010001111110000000000"), state = addBomb<1>(state, 1ULL << (23 * 2)));
	assert_equal(State<6,5>::parse("011011222320103211111110001120", "010000101010001111110000000000"), state = addBomb<0>(state, 1ULL << (1  * 2)));
	assert_equal(State<6,5>::parse("111011222320103211111110001120", "110000101010001111110000000000"), state = addBomb<1>(state, 1ULL << (29 * 2)));
	assert_equal(State<6,5>::parse("111011222321103211111110001120", "110000101010001111110000000000"), state = addBomb<0>(state, 1ULL << (18 * 2)));
	assert_equal(State<6,5>::parse("111111222321103211111110001120", "110100101010001111110000000000"), state = addBomb<1>(state, 1ULL << (26 * 2)));
	assert_equal(State<6,5>::parse("111111222321103211112110001120", "110100101010001111110000000000"), state = addBomb<0>(state, 1ULL << (9  * 2)));
	assert_equal(State<6,5>::parse("111111223321103211112110001120", "110100101010001111110000000000"), state = addBomb<1>(state, 1ULL << (21 * 2)));
	assert_equal(State<6,5>::parse("112111223321103211112110001120", "110100101010001111110000000000"), state = addBomb<0>(state, 1ULL << (27 * 2)));
	assert_equal(State<6,5>::parse("112111223321103311112110001120", "110100101010001111110000000000"), state = addBomb<1>(state, 1ULL << (14 * 2)));
	assert_equal(State<6,5>::parse("112111223321103311112110101120", "110100101010001111110000000000"), state = addBomb<0>(state, 1ULL << (5  * 2)));
	assert_equal(State<6,5>::parse("112111223321113311112110101120", "110100101010011111110000000000"), state = addBomb<1>(state, 1ULL << (16 * 2)));
	assert_equal(State<6,5>::parse("112111223321113311112110101220", "110100101010011111110000000000"), state = addBomb<0>(state, 1ULL << (2  * 2)));
	assert_equal(State<6,5>::parse("112111223321113312112110101220", "110100101010011111110000000000"), state = addBomb<1>(state, 1ULL << (12 * 2)));
	assert_equal(State<6,5>::parse("112111223321113312112110101221", "110100101010011111110000000000"), state = addBomb<0>(state, 1ULL << (0  * 2)));
	assert_equal(State<6,5>::parse("112111223321123312112110101221", "110100101010011111110000000000"), state = addBomb<1>(state, 1ULL << (16 * 2)));
	assert_equal(State<6,5>::parse("112120223322123312112110101221", "110100101010011111110000000000"), state = addBomb<0>(state, 1ULL << (24 * 2)));
	assert_equal(State<6,5>::parse("112120223322123312112111101221", "110100101010011111110001000000"), state = addBomb<1>(state, 1ULL << (6  * 2)));
	assert_equal(State<6,5>::parse("112120223322123312112111102221", "110100101010011111110001000000"), state = addBomb<0>(state, 1ULL << (3  * 2)));
	assert_equal(State<6,5>::parse("022120133322223312112111102221", "010100111010111111110001000000"), state = addBomb<1>(state, 1ULL << (23 * 2)));
	assert_equal(State<6,5>::parse("022120133322223312112121102221", "010100111010111111110001000000"), state = addBomb<0>(state, 1ULL << (7  * 2)));
	assert_equal(State<6,5>::parse("022120233322033312212121102221", "010100111010011111110001000000"), state = addBomb<1>(state, 1ULL << (17 * 2)));
	assert_equal(State<6,5>::parse("112120212311223312223232102221", "000000000000000000100000000000"), state = addBomb<0>(state, 1ULL << (20 * 2)));
	assert_equal(State<6,5>::parse("022120122311133312233232012221", "010000110000110000110000010000"), state = addBomb<1>(state, 1ULL << (11 * 2)));
	// game ends with player 0 putting at cellid 3
	assert_equal(~0ULL, (state = addBomb<1>(state, 1ULL << (3 * 2))).players);
	std::cout << "All tests passed" << std::endl;
}