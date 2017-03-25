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
	int eval;
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
	Tensor OutputEvalTensor;
	uint64_t BatchSize;

	Engine();
	~Engine();

	Move go_alphabeta();
	Move go_negamax();
	int AlphaBeta(int alpha, int beta, int depth);
	int Negamax(int depth);

	int LeafEval();
	int QuiescenceSearch();

	void learn_eval(int num_games);

	uint64_t perft(int depth);
};

