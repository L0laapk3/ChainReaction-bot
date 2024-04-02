#include "boardStep.hpp"
#include "tests_critical.hpp"
#include "search.hpp"





template<typename T>
constexpr void assert_equal(T a, T b) {
	if (a != b) {
		std::cout << a << " != " << b << std::endl;
		throw std::runtime_error("assertion failed");
	}
}

int main(int, char**) {
	constexpr size_t W = 6, H = 5;
	{
		auto state = defaultState<W,H>;
		state.place<1>(1ULL << (13 * 2));	assert_equal(state, State<W,H>::parse("000000000000000010000000000000", "000000000000000010000000000000"));
		state.place<0>(1ULL << (24 * 2));	assert_equal(state, State<W,H>::parse("000001000000000010000000000000", "000000000000000010000000000000"));
		state.place<1>(1ULL << (12 * 2));	assert_equal(state, State<W,H>::parse("000001000000000011000000000000", "000000000000000011000000000000"));
		state.place<0>(1ULL << (9  * 2));	assert_equal(state, State<W,H>::parse("000001000000000011001000000000", "000000000000000011000000000000"));
		state.place<1>(1ULL << (15 * 2));	assert_equal(state, State<W,H>::parse("000001000000001011001000000000", "000000000000001011000000000000"));
		state.place<0>(1ULL << (2  * 2));	assert_equal(state, State<W,H>::parse("000001000000001011001000000100", "000000000000001011000000000000"));
		state.place<1>(1ULL << (14 * 2));	assert_equal(state, State<W,H>::parse("000001000000001111001000000100", "000000000000001111000000000000"));
		state.place<0>(1ULL << (22 * 2));	assert_equal(state, State<W,H>::parse("000001010000001111001000000100", "000000000000001111000000000000"));
		state.place<1>(1ULL << (10 * 2));	assert_equal(state, State<W,H>::parse("000001010000001111011000000100", "000000000000001111010000000000"));
		state.place<0>(1ULL << (1  * 2));	assert_equal(state, State<W,H>::parse("000001010000001111011000000110", "000000000000001111010000000000"));
		state.place<1>(1ULL << (15 * 2));	assert_equal(state, State<W,H>::parse("000001010000002111011000000110", "000000000000001111010000000000"));
		state.place<0>(1ULL << (8  * 2));	assert_equal(state, State<W,H>::parse("000001010000002111011100000110", "000000000000001111010000000000"));
		state.place<1>(1ULL << (21 * 2));	assert_equal(state, State<W,H>::parse("000001011000002111011100000110", "000000001000001111010000000000"));
		state.place<0>(1ULL << (25 * 2));	assert_equal(state, State<W,H>::parse("000011011000002111011100000110", "000000001000001111010000000000"));
		state.place<1>(1ULL << (23 * 2));	assert_equal(state, State<W,H>::parse("000011111000002111011100000110", "000000101000001111010000000000"));
		state.place<0>(1ULL << (20 * 2));	assert_equal(state, State<W,H>::parse("000011111100002111011100000110", "000000101000001111010000000000"));
		state.place<1>(1ULL << (19 * 2));	assert_equal(state, State<W,H>::parse("000011111110002111011100000110", "000000101010001111010000000000"));
		state.place<0>(1ULL << (7  * 2));	assert_equal(state, State<W,H>::parse("000011111110002111011110000110", "000000101010001111010000000000"));
		state.place<1>(1ULL << (21 * 2));	assert_equal(state, State<W,H>::parse("000011112110002111011110000110", "000000101010001111010000000000"));
		state.place<0>(1ULL << (3  * 2));	assert_equal(state, State<W,H>::parse("000011112110002111011110001110", "000000101010001111010000000000"));
		state.place<1>(1ULL << (14 * 2));	assert_equal(state, State<W,H>::parse("000011112110002211011110001110", "000000101010001111010000000000"));
		state.place<0>(1ULL << (27 * 2));	assert_equal(state, State<W,H>::parse("001011112110002211011110001110", "000000101010001111010000000000"));
		state.place<1>(1ULL << (15 * 2));	assert_equal(state, State<W,H>::parse("001011112110003211011110001110", "000000101010001111010000000000"));
		state.place<0>(1ULL << (22 * 2));	assert_equal(state, State<W,H>::parse("001011122110003211011110001110", "000000101010001111010000000000"));
		state.place<1>(1ULL << (11 * 2));	assert_equal(state, State<W,H>::parse("001011122110003211111110001110", "000000101010001111110000000000"));
		state.place<0>(1ULL << (20 * 2));	assert_equal(state, State<W,H>::parse("001011122210003211111110001110", "000000101010001111110000000000"));
		state.place<1>(1ULL << (28 * 2));	assert_equal(state, State<W,H>::parse("011011122210003211111110001110", "010000101010001111110000000000"));
		state.place<0>(1ULL << (20 * 2));	assert_equal(state, State<W,H>::parse("011011122310003211111110001110", "010000101010001111110000000000"));
		state.place<1>(1ULL << (19 * 2));	assert_equal(state, State<W,H>::parse("011011122320003211111110001110", "010000101010001111110000000000"));
		state.place<0>(1ULL << (17 * 2));	assert_equal(state, State<W,H>::parse("011011122320103211111110001110", "010000101010001111110000000000"));
		state.place<1>(1ULL << (23 * 2));	assert_equal(state, State<W,H>::parse("011011222320103211111110001110", "010000101010001111110000000000"));
		state.place<0>(1ULL << (1  * 2));	assert_equal(state, State<W,H>::parse("011011222320103211111110001120", "010000101010001111110000000000"));
		state.place<1>(1ULL << (29 * 2));	assert_equal(state, State<W,H>::parse("111011222320103211111110001120", "110000101010001111110000000000"));
		state.place<0>(1ULL << (18 * 2));	assert_equal(state, State<W,H>::parse("111011222321103211111110001120", "110000101010001111110000000000"));
		state.place<1>(1ULL << (26 * 2));	assert_equal(state, State<W,H>::parse("111111222321103211111110001120", "110100101010001111110000000000"));
		state.place<0>(1ULL << (9  * 2));	assert_equal(state, State<W,H>::parse("111111222321103211112110001120", "110100101010001111110000000000"));
		state.place<1>(1ULL << (21 * 2));	assert_equal(state, State<W,H>::parse("111111223321103211112110001120", "110100101010001111110000000000"));
		state.place<0>(1ULL << (27 * 2));	assert_equal(state, State<W,H>::parse("112111223321103211112110001120", "110100101010001111110000000000"));
		state.place<1>(1ULL << (14 * 2));	assert_equal(state, State<W,H>::parse("112111223321103311112110001120", "110100101010001111110000000000"));
		state.place<0>(1ULL << (5  * 2));	assert_equal(state, State<W,H>::parse("112111223321103311112110101120", "110100101010001111110000000000"));
		state.place<1>(1ULL << (16 * 2));	assert_equal(state, State<W,H>::parse("112111223321113311112110101120", "110100101010011111110000000000"));
		state.place<0>(1ULL << (2  * 2));	assert_equal(state, State<W,H>::parse("112111223321113311112110101220", "110100101010011111110000000000"));
		state.place<1>(1ULL << (12 * 2));	assert_equal(state, State<W,H>::parse("112111223321113312112110101220", "110100101010011111110000000000"));
		state.place<0>(1ULL << (0  * 2));	assert_equal(state, State<W,H>::parse("112111223321113312112110101221", "110100101010011111110000000000"));
		state.place<1>(1ULL << (16 * 2));	assert_equal(state, State<W,H>::parse("112111223321123312112110101221", "110100101010011111110000000000"));
		state.place<0>(1ULL << (24 * 2));	assert_equal(state, State<W,H>::parse("112120223322123312112110101221", "110100101010011111110000000000"));
		state.place<1>(1ULL << (6  * 2));	assert_equal(state, State<W,H>::parse("112120223322123312112111101221", "110100101010011111110001000000"));
		state.place<0>(1ULL << (3  * 2));	assert_equal(state, State<W,H>::parse("112120223322123312112111102221", "110100101010011111110001000000"));
		state.place<1>(1ULL << (23 * 2));	assert_equal(state, State<W,H>::parse("022120133322223312112111102221", "010100111010111111110001000000"));
		state.place<0>(1ULL << (7  * 2));	assert_equal(state, State<W,H>::parse("022120133322223312112121102221", "010100111010111111110001000000"));
		state.place<1>(1ULL << (17 * 2));	assert_equal(state, State<W,H>::parse("022120233322033312212121102221", "010100111010011111110001000000"));
		state.place<0>(1ULL << (20 * 2));	assert_equal(state, State<W,H>::parse("112120212311223312223232102221", "000000000000000000100000000000"));
		state.place<1>(1ULL << (11 * 2));	assert_equal(state, State<W,H>::parse("022120122311133312233232012221", "010000110000110000110000010000"));
		// game ends with player 0 putting at cellid 3
		state.place<0>(1ULL << (3  * 2));
		assert_equal<board_t<W,H>>(0ULL, state.players);
		std::cout << "All tests passed" << std::endl;
	}

	auto state = defaultState<W,H>;
	while (true) {
		auto result = search(state, { .time = 100 });
		if (!result.foundMove) {
			std::cout << "No moves found" << std::endl;
			break;
		}

		state.place<0>(1ULL << (result.bestMove * 2));
		std::cout << state << std::endl;
		std::cout << result.bestMove << std::endl;
		if (state.isWon()) {
			std::cout << "The end :)" << std::endl;
			break;
		}
		state.invertPlayer();
		if (result.bestMove < 0 || result.bestMove >= 6*5)
			throw std::runtime_error("Game fucked");
	}
}