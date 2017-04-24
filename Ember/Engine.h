#pragma once

#include "SortNet.h"

//#define TRAINING_BUILD
#define DO_NULL_MOVE

extern jmp_buf JumpEnv;

extern const int CONST_INF; 
extern const int DATABASE_MAX_SIZE;

enum TimeMode { MODE_DEFAULT, MODE_MOVETIME, MODE_INF, MODE_DEPTH };

struct Data
{
public:
	Data() : move(make_shape(MOVE_TENSOR_SIZE)) {}
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

	EvalNet* EvalNet_Play;
	EvalNet* EvalNet_Train;
	SortNet* SortNet_Play;
	SortNet* SortNet_Train;

	PositionNN PosNN;
	Tensor MoveNN;
	Tensor PositionTensor;
	Tensor OutputMoveTensor;
	Tensor OutputEvalTensor;
	uint64_t BatchSize;

	Tensor MoveTensor;
	Tensor SortTensor;
	uint64_t SortNetCount;

	Move KillerMoves[2][100];
	long HistoryScores[64][64];

	TranspositionTable* Table;

	uint64_t NodeCount;
	uint64_t BetaCutoffCount;
	uint64_t BetaCutoffValue;

	size_t CumulativeCount;
	Float CumulativeSum;

	Engine();
	~Engine();

	void load_evalnets(std::string path);
	void load_sortnets(std::string path);

	//Search
	SearchResult go(int mode, int wtime, int btime, int winc, int binc, bool print);

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
	void sortNet_forward(std::vector<Move>& moves);

	//QSearch
	int QSearch(int alpha, int beta);
	int StaticExchangeEvaluation(int to, int from, int movpiece, int capt);

	//Learning
	void learn_eval(uint64_t num_games);
	void learn_eval_NN(uint64_t num_games, double time_limit);
	void learn_eval_TD(uint64_t num_games, double time_limit);
	void learn_eval_TD_pgn(const PGNData& pgn, double time_limit);
	void learn_eval_pgn(const PGNData& pgn, double time_limit);


	void checkup()
	{
		Timer.Stop();
		if (Timer.ElapsedMilliseconds() >= AllocatedTime)
		{
			longjmp(JumpEnv, Timer.ElapsedMilliseconds());
		}
	}

	bool isDraw();
};

