#pragma once

#include "types.h"
#include "score.h"
#include "boardEval.hpp"
#include "boardIter.hpp"

#include <chrono>



struct RootResult {
	Score score;
	size_t bestMove;
	bool foundMove;
	explicit operator Score() const { return score; };
};
struct SearchResult : public RootResult {
	S64 durationUs;
	Depth depth;
};


template<bool root, bool quiescence, bool penalizeDistance, size_t W, size_t H>
std::conditional_t<root, RootResult, Score> negamax(State<W,H> state, Score alpha, Score beta, Depth remainingDepth) {
	Score alphaOrig = alpha;

	if constexpr (!root && quiescence) {
		Score standingPat = state.evaluate();
		if (standingPat >= beta)
			return standingPat;
		if (alpha < standingPat)
			alpha = standingPat;
	}

	size_t ttBestMove = -1ULL;
	if constexpr (!quiescence) {
		// TT lookup
	}


	size_t bestMove = -1ULL;
	size_t bestRootMove = -1ULL;
	bool foundMove = false;
	Score bestRootScore = SCORE::MIN;
	state.template iterateMoves<!root && quiescence>([&](size_t move) {
		auto newState = state;
		// std::cout << "before" << std::endl << state;
		newState.place(1ULL << (2 * move));
		// std::cout << "after" << std::endl << state;
		foundMove = true;
		Score score;

		if ((!root || state.countBombs() > 1) && newState.isWon())
			score = SCORE::WIN;
		else {

			newState.validate();
			newState.invertPlayer();
			// std::cout << "afterer" << std::endl << state << std::endl;
			newState.validate();

			if (quiescence || remainingDepth - 1 <= 1) // trackDistance: widen the search window so we can subtract one again to penalize for distance
				score = -negamax<false, true,  penalizeDistance>(newState, -(beta + (beta >= 0 ? penalizeDistance : -penalizeDistance)), -(alpha + (alpha >= 0 ? penalizeDistance : -penalizeDistance)), 1);
			else
				score = -negamax<false, false, penalizeDistance>(newState, -(beta + (beta >= 0 ? penalizeDistance : -penalizeDistance)), -(alpha + (alpha >= 0 ? penalizeDistance : -penalizeDistance)), remainingDepth - 1);

			if (penalizeDistance && score != 0) // move score closer to zero for every move
				score -= score >= 0 ? 1 : -1;
		}

		if (root && score > bestRootScore) {
			bestRootScore = score;
			bestRootMove = move;
		}

		if (score > alpha) {
			alpha = score;
			bestMove = move;
			if (alpha >= beta)
				return true;
		}
		return false;
	});

	if (!quiescence && !root) {
		// TT store
	}

	return (std::conditional_t<root, RootResult, Score>)RootResult{ alpha, bestRootMove, foundMove };
}



template<size_t W, size_t H>
SearchResult searchDepth(State<W,H> state, Depth depth, bool searchWin, Score alpha, Score beta) {
	SearchResult result{};
	result.depth = depth;
	auto start = std::chrono::high_resolution_clock::now();
	if (!searchWin)
		(RootResult&)result = negamax<true, false, false>(state, alpha, beta, depth + 1);
	else
		(RootResult&)result = negamax<true, false, true> (state, alpha, beta, depth + 1);
	auto end = std::chrono::high_resolution_clock::now();
	result.durationUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	if (!result.foundMove)
		std::cerr << "No moves found" << std::endl;
	return result;
}

struct SearchStopCriteria {
	S64 time = 1000 * 60 * 24 * 365;
	Depth depth  = DEPTH_MAX;
};

struct SearchPersistent {
	Score alpha = -MUL_PIECE_ADVANTAGE / 10;
	Score beta  =  MUL_PIECE_ADVANTAGE / 10;
	Score score = 0;
	Depth depth = 0;
	bool searchWin = false;
};

template<size_t W, size_t H>
SearchResult search(State<W,H> state, SearchStopCriteria stop, SearchPersistent& persistent) {
	SearchResult result{};
	ScoreParsed parsedScore{}, lastParsed = parseScore(persistent.score);

	bool searchWin = lastParsed.outcome != SCORE::DRAW;
	if (searchWin != persistent.searchWin) {
		persistent.searchWin = searchWin;
		// TT: recalculate
	}

	S64 lastDurationUs = 1, lastDurationUs2 = 1;
	bool widenedAspirationWindow = false;
	Depth depth = 1;
	S64 usedTime = 0;
	stop.time *= 1000;
	while (depth < stop.depth) {
		depth++;
		while (true) {
			while (true) {
				(SearchResult&)result = searchDepth(state, depth, persistent.searchWin, persistent.alpha, persistent.beta);
				usedTime += result.durationUs;
				parsedScore = parseScore(result.score);
				if ((parsedScore.outcome != SCORE::DRAW) && !persistent.searchWin) {
					persistent.searchWin = parsedScore.outcome != SCORE::DRAW;
					// TT: recalculate
					continue;
				}

				if (parsedScore.outcome != SCORE::DRAW && parsedScore.outcomeDistance <= depth + 1) {
					persistent.depth = parsedScore.outcomeDistance - 2; // limit next search depth to mate distance
					std::cout << "stop at " << persistent.depth << std::endl;
					goto stopSearchNoDepthSet;
				}
				break;
			}


			if (result.score <= persistent.alpha) {
				if (!widenedAspirationWindow && persistent.alpha > -MUL_PIECE_ADVANTAGE)
					persistent.alpha -= MUL_PIECE_ADVANTAGE;
				else {
					persistent.alpha = SCORE::LOSS;
				}
				widenedAspirationWindow = true;
			} else if (result.score >= persistent.beta) {
				if (!widenedAspirationWindow && persistent.beta < MUL_PIECE_ADVANTAGE)
					persistent.beta += MUL_PIECE_ADVANTAGE;
				else {
					persistent.beta = SCORE::WIN;
				}
				widenedAspirationWindow = true;
			} else
				break;
			printf("new window: [%s, %s] %s\n", scoreToString(persistent.alpha).c_str(), scoreToString(persistent.beta).c_str(), scoreToString(result.score).c_str());
			// TT: recalculate
		}

		if (result.durationUs > stop.time - usedTime)
			break;
	}
	persistent.depth = depth;
stopSearchNoDepthSet:
	persistent.depth--; // next search: one less depth
	printf("Depth: %2d, Score: %s, Time: %ldms\n", depth, scoreToString(result.score).c_str(), usedTime / 1000);
	if (parsedScore.outcome != SCORE::DRAW && parsedScore.outcomeDistance < depth && depth > 2)
		throw std::runtime_error("Won found in less depth than search depth");

	result.depth = depth;
	return result;
}

template<size_t W, size_t H>
SearchResult search(State<W,H> state, SearchStopCriteria stop) {
	SearchPersistent persistent{};
	return search(state, stop, persistent);
}