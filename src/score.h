#pragma once

#include <limits>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>



typedef int16_t Depth;

constexpr Depth DEPTH_MAX = 512;

typedef int Score;
enum SCORE : int {
	MAX  = std::numeric_limits<int>::max() - DEPTH_MAX * 2,
	WIN  = MAX - DEPTH_MAX * 2,
	DRAW = 0,
	LOSS = -WIN,
	MIN  = -MAX,
};


constexpr Score MUL_POSITION_ADVANTAGE = 1024;
constexpr Score MUL_PIECE_ADVANTAGE    = 1024 * MUL_POSITION_ADVANTAGE;


struct ScoreParsed {
	Score outcome; // win, lose, 0
	Depth outcomeDistance;
	Score eval; // score of the position
};

inline ScoreParsed parseScore(Score score) {
	ScoreParsed result{};
	result.eval = score;
	if (score >= SCORE::WIN - DEPTH_MAX) {
		result.outcome = SCORE::WIN;
		result.outcomeDistance = SCORE::WIN - score + 1;
	} else if (score <= SCORE::LOSS + DEPTH_MAX) {
		result.outcome = SCORE::LOSS;
		result.outcomeDistance = score - SCORE::LOSS + 1;
	} else
		result.outcome = SCORE::DRAW;
	return result;
}
inline std::string scoreToString(ScoreParsed parsed, bool player = 0) {
	if (parsed.outcome != SCORE::DRAW)
		return std::string((parsed.outcome == SCORE::WIN) != player ? " win" : "lose") + " in " + std::to_string(parsed.outcomeDistance);

	if (player)
		parsed.eval *= -1;

	std::stringstream stream;
	stream << (parsed.eval > 0 ? "+" : parsed.eval == SCORE::DRAW ? " " : "") << std::fixed << std::setprecision(4) << ((double)parsed.eval / MUL_PIECE_ADVANTAGE);
	return stream.str();
}
inline std::string scoreToString(Score score, bool player = 0) {
	return scoreToString(parseScore(score), player);
}