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
	PositionNN PosNN;
	Tensor InputTensor;
	Tensor OutputTensor;
	Tensor OutputEvalTensor;
	uint64_t BatchSize;

	Move KillerMoves[2][100];
	long HistoryScores[64][64];

	TranspositionTable* Table;

	Engine();
	~Engine();

	Move go(int mode, int wtime, int btime, int winc, int binc, bool print);

	GoReturn go_alphabeta(int depth);
	Move go_negamax(int depth);
	int AlphaBeta(int alpha, int beta, int depth, int ply);
	int Negamax(int depth, int ply);

	int LeafEval();
	int LeafEval_MatOnly();
	int LeafEval_NN();
	
	int getMoveScore(const Move& m, int ply);
	Move getNextMove(std::vector<Move>& moves, int current_move, int ply);
	void setKiller(const Move& m, int ply);

	void learn_eval(int num_games);
	void learn_eval_NN(int num_games);

	uint64_t perft(int depth);
};

