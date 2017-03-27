#pragma once

#include "Net.h"

extern const int CONST_INF;

enum TimeMode { MODE_DEFAULT, MODE_MOVETIME, MODE_INF, MODE_DEPTH };

struct Data
{
public:
	Data() : move(make_shape(20, 2, 64)) {}
	~Data() { move.freemem(); }

	PositionNN pos;
	Tensor move;
	int eval;
};

struct GoReturn
{
	GoReturn(Move move, int score) : m(move), eval(score) {}

	Move m;
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

	Move go(int mode, int wtime, int btime, int winc, int binc, bool print);

	GoReturn go_alphabeta();
	Move go_negamax();
	int AlphaBeta(int alpha, int beta, int depth);
	int Negamax(int depth);

	int LeafEval();
	int QuiescenceSearch();

	void learn_eval(int num_games);
	void learn_eval_NN(int num_games);

	uint64_t perft(int depth);
};

