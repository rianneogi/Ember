#pragma once

#include "Net.h"

extern const int CONST_INF;

struct Data
{
public:
	Data();
	~Data();

	PositionNN pos;
	Tensor move;
	int depth;
};

class Engine
{
public:
	Position CurrentPos;
	Data* Database;
	size_t DBCounter;
	size_t DBSize;

	Net* mNet;
	Tensor InputTensor;
	Tensor OutputTensor;
	uint64_t BatchSize;

	Engine();
	~Engine();

	void go();
	int AlphaBeta(int alpha, int beta, int depth);
	int Negamax(int depth);

	int LeafEval();
	int QuiescenceSearch();

	uint64_t perft(int depth);
};

