#include "Engine.h"

const int CONST_INF = 10000;

Engine::Engine()
{
}

Engine::~Engine()
{
}

void Engine::go()
{
}

int Engine::AlphaBeta(int alpha, int beta, int depth)
{
	if (depth == 0)
		return LeafEval();

	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);
	int bestscore = -CONST_INF;
	for (int i = 0; i < moves.size(); i++)
	{
		CurrentPos.makeMove(moves[i]);
		int score = -AlphaBeta(-beta, -alpha, depth - 1);
		CurrentPos.unmakeMove(moves[i]);

		if (score >= beta)
		{
			return beta;
		}
		else if (score > bestscore)
		{
			if (score > alpha)
			{
				alpha = score;
			}
			bestscore = score;
		}
	}

	return bestscore;
}

int Engine::Negamax(int depth)
{
	if (depth == 0)
		return LeafEval();

	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);
	int bestscore = -CONST_INF;
	for (int i = 0; i < moves.size(); i++)
	{
		CurrentPos.makeMove(moves[i]);
		int score = -Negamax(depth - 1);
		CurrentPos.unmakeMove(moves[i]);

		if (score > bestscore)
		{
			bestscore = score;
		}
	}
	return bestscore;
}

int Engine::LeafEval()
{
	int score[2] = { 0,0 };
	for (int i = 0; i < 2; i++)
	{
		score[i] += 100 * popcnt(CurrentPos.Pieces[i][PIECE_PAWN]);
		score[i] += 320 * popcnt(CurrentPos.Pieces[i][PIECE_KNIGHT]);
		score[i] += 350 * popcnt(CurrentPos.Pieces[i][PIECE_BISHOP]);
		score[i] += 500 * popcnt(CurrentPos.Pieces[i][PIECE_ROOK]);
		score[i] += 950 * popcnt(CurrentPos.Pieces[i][PIECE_QUEEN]);
	}
	int ret = score[0] - score[1];
	if (CurrentPos.Turn == COLOR_BLACK)
	{
		ret = -ret;
	}
	return ret;
}

uint64_t Engine::perft(int depth)
{
	if (depth == 0) return 1;

	if (CurrentPos.getGameStatus() != STATUS_NOTOVER)
	{
		return 0;
	}

	uint64_t count = 0;
	std::vector<Move> vec;
	vec.reserve(128);
	CurrentPos.generateMoves(vec);

	auto Key = CurrentPos.HashKey;
	auto os = CurrentPos.OccupiedSq;

	for (unsigned int i = 0; i < vec.size(); i++)
	{
		Move m = vec[i];
		CurrentPos.makeMove(m);
		count += perft(depth-1);
		CurrentPos.unmakeMove(m);
	}

	assert(Key == CurrentPos.HashKey);
	assert(os == CurrentPos.OccupiedSq);

	return count;
}
