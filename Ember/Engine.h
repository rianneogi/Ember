#pragma once

#include "Net.h"

extern const int CONST_INF;

struct Data
{
	PositionNN pos;
	Move move;
	int depth;
};

class Engine
{
public:
	Position CurrentPos;
	Data* Database;
	unsigned int DBCounter;

	Engine();
	~Engine();

	void go();
	int AlphaBeta(int alpha, int beta, int depth);
	int Negamax(int depth);

	int LeafEval();
	int QuiescenceSearch();

	uint64_t perft(int depth);
};

