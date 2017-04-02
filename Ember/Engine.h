#pragma once

#include "SortNet.h"

//#define TRAINING_BUILD

extern jmp_buf JumpEnv;

extern const int CONST_INF; 
extern const int DATABASE_MAX_SIZE;

enum TimeMode { MODE_DEFAULT, MODE_MOVETIME, MODE_INF, MODE_DEPTH };

struct Data
{
public:
	Data() : move(make_shape(20, 2, 64)) {}
	~Data() { move.freemem(); }

	PositionNN pos;
	Tensor move;
	Float eval;
};

struct SearchResult
{
	SearchResult(Move move, int score) : m(move), eval(score) {}

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

	Clock Timer;
	uint64_t AllocatedTime;
	int TimeMode;

	Net* NetPlay;
	Net* NetTrain;
	SortNet* NetSort;

	PositionNN PosNN;
	Tensor InputTensor;
	Tensor OutputMoveTensor;
	Tensor OutputEvalTensor;
	uint64_t BatchSize;

	Tensor MoveTensor;
	Tensor SortTensor;

	Move KillerMoves[2][100];
	long HistoryScores[64][64];

	TranspositionTable* Table;

	uint64_t NodeCount;

	Engine();
	~Engine();

	void load_nets(std::string path);

	//Search
	Move go(int mode, int wtime, int btime, int winc, int binc, bool print);

	SearchResult go_alphabeta(int depth);
	Move go_negamax(int depth);
	int AlphaBeta(int alpha, int beta, int depth, int ply);
	int Negamax(int depth, int ply);

	uint64_t perft(int depth);

	//Evaluation
	int LeafEval();
	int LeafEval_MatOnly();
	Float LeafEval_NN();
	
	//MoveSort
	int getMoveScore(const Move& m, int ply);
	Move getNextMove(std::vector<Move>& moves, int current_move, int ply);
	void setKiller(const Move& m, int ply);
	Move getNextMove_NN(std::vector<Move>& moves, int current_move, int ply);

	//Learning
	void learn_eval(uint64_t num_games);
	void learn_eval_NN(uint64_t num_games, double time_limit);
	void learn_eval_TD(uint64_t num_games, double time_limit);
	void updateVariables_TD(uint64_t start, uint64_t batch_size);


	void checkup()
	{
		Timer.Stop();
		if (Timer.ElapsedMilliseconds() >= AllocatedTime)
		{
			longjmp(JumpEnv, Timer.ElapsedMilliseconds());
		}
	}
};

