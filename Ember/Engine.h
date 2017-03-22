#pragma once

#include "Position.h"

extern const int CONST_INF;

class Engine
{
public:
	Position CurrentPos;

	Engine();
	~Engine();

	void go();
	int AlphaBeta(int alpha, int beta, int depth);
	int Negamax(int depth);

	int LeafEval();
	int QuiescenceSearch();

	uint64_t perft(int depth);
};

