#include "Engine.h"

const int DATABASE_SIZE = 6400;
const int CONST_INF = 10000;

const int BATCH_SIZE = 64;

Engine::Engine()
	: BatchSize(BATCH_SIZE), InputTensor(make_shape(BATCH_SIZE, 8, 8, 14)), 
	OutputTensor(make_shape(BATCH_SIZE, 2, 64)), OutputEvalTensor(make_shape(BATCH_SIZE, 1))
{
	Database = new Data[DATABASE_SIZE];
	DBCounter = 0;
	DBSize = 0;
	mNet = new Net(BatchSize);
}

Engine::~Engine()
{
	delete[] Database;
	delete mNet;
	Database = NULL;
	mNet = NULL;
}

Move Engine::go(int mode, int wtime, int btime, int winc, int binc, bool print)
{
	GoReturn go = go_alphabeta();
	if (print)
	{
		std::cout << "info score cp " << go.eval << std::endl;
	}
	return go_alphabeta().m;
}

GoReturn Engine::go_alphabeta()
{
	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);
	int bestscore = -CONST_INF;
	Move bestmove = createNullMove(CurrentPos.EPSquare);
	//assert(popcnt(CurrentPos.Pieces[COLOR_WHITE][PIECE_KING]) != 0);
	//assert(popcnt(CurrentPos.Pieces[COLOR_BLACK][PIECE_KING]) != 0);
	//assert(CurrentPos.underCheck(getOpponent(CurrentPos.Turn)) == false);
	for (size_t i = 0; i < moves.size(); i++)
	{
		//printf("Move: %s\n", moves[i].toString());
		CurrentPos.makeMove(moves[i]);
		int score = -AlphaBeta(-CONST_INF, CONST_INF, 3);
		CurrentPos.unmakeMove(moves[i]);

		if (score >= bestscore)
		{
			bestscore = score;
			bestmove = moves[i];
		}
	}
	return GoReturn(bestmove, bestscore);
}

Move Engine::go_negamax()
{
	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);
	int bestscore = -CONST_INF;
	Move bestmove = createNullMove(CurrentPos.EPSquare);
	for (size_t i = 0; i < moves.size(); i++)
	{
		//printf("Move: %s\n", moves[i].toString());

		CurrentPos.makeMove(moves[i]);
		int score = -Negamax(4);
		CurrentPos.unmakeMove(moves[i]);

		if (score > bestscore)
		{
			bestscore = score;
			bestmove = moves[i];
		}
	}
	return bestmove;
}

int Engine::AlphaBeta(int alpha, int beta, int depth)
{
	if (depth == 0)
		return LeafEval();

	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);

	//assert(popcnt(CurrentPos.Pieces[COLOR_WHITE][PIECE_KING]) != 0);
	//assert(popcnt(CurrentPos.Pieces[COLOR_BLACK][PIECE_KING]) != 0);
	//assert(CurrentPos.underCheck(getOpponent(CurrentPos.Turn)) == false);

	if (moves.size() == 0)
	{
		int status = CurrentPos.getGameStatus();
		if (status != STATUS_NOTOVER)
		{
			if (status == STATUS_STALEMATE || status == STATUS_INSUFFICIENTMAT || status == STATUS_3FOLDREP)
			{
				return 0;
			}
			else if (status == STATUS_WHITEMATED || status == STATUS_BLACKMATED)
			{
				return -CONST_INF;
			}
		}
	}

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

	int leafeval = LeafEval();

	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);

	if (moves.size() == 0)
	{
		int status = CurrentPos.getGameStatus();
		if (status != STATUS_NOTOVER)
		{
			if (status == STATUS_STALEMATE || status == STATUS_INSUFFICIENTMAT || status == STATUS_3FOLDREP)
			{
				return 0;
			}
			else if (status == STATUS_WHITEMATED || status == STATUS_BLACKMATED)
			{
				return -CONST_INF;
			}
		}
	}

	int bestscore = -CONST_INF;
	Move bestmove;
	for (size_t i = 0; i < moves.size(); i++)
	{
		CurrentPos.makeMove(moves[i]);
		int score = -Negamax(depth - 1);
		CurrentPos.unmakeMove(moves[i]);

		if (score > bestscore)
		{
			bestscore = score;
			bestmove = moves[i];
		}
	}

	if (bestscore != leafeval && depth>=4)
	{
		Data* d = &Database[DBCounter];
		d->pos.copyFromPosition(CurrentPos);
		d->eval = leafeval;
		moveToTensor(bestmove, &d->move);

		DBCounter++;
		if (DBCounter == DATABASE_SIZE)
		{
			DBCounter = 0;
		}

		if (DBSize < DATABASE_SIZE)
		{
			DBSize++;
		}

		for (uint64_t i = 0; i < BatchSize; i++)
		{
			size_t id = rand() % DBSize;
			memcpy(&InputTensor(i * 14 * 8 * 8), &Database[id].pos.Squares.mData, sizeof(Float) * 14 * 8 * 8);
			memcpy(&OutputTensor(i * 2 * 8), &Database[id].move.mData, sizeof(Float) * 2 * 64);
			OutputEvalTensor(i) = Database[id].eval;
		}
		mNet->train(InputTensor, nullptr, &OutputEvalTensor);
	}

	return bestscore;
}

int PieceSqValues[6][64] =
{
	//{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //empty
	{ 0,  0,  0,  0,  0,  0,  0,  0, //pawn
	-12, -4, -4,  0,  0, -4, -4,-12,
	-12, -4,  8,  8,  8,  8, -4,-12,
	-8,  8, 24, 32, 32, 24,  8, -8,
	-8,  8,  8, 16, 16,  8,  8, -8,
	-8,  0,  0,  0,  0,  0,  0, -8,
	-8,  0,  0,  0,  0,  0,  0, -8,
	0,  0,  0,  0,  0,  0,  0,  0 },

	{ 2,  4,  6,  6,  6,  6,  4,  2, //knight, non-negative
	4,  6, 10, 12, 12, 10,  6,  4,
	6, 14, 20, 20, 20, 20, 14,  6,
	10, 18, 22, 20, 20, 22, 18, 10,
	10, 16, 18, 18, 18, 18, 16, 10,
	10, 18, 24, 20, 20, 24, 18, 10,
	8, 12, 20, 22, 22, 20, 12,  8,
	4,  8, 10, 12, 12, 10,  8,  4 },

	{ 16, 10,  6,  6,  6,  6, 10, 16, //bishop, non-negative
	14, 16, 12, 10, 10, 12, 16, 14,
	10, 14, 18, 14, 14, 18, 14, 10,
	16, 12, 18, 22, 22, 18, 12, 16,
	16, 12, 14, 18, 18, 14, 12, 16,
	16, 14, 18, 18, 18, 18, 14, 16,
	10, 18, 16, 14, 14, 16, 18, 10,
	14, 14, 10, 10, 10, 10, 14, 14 },

	{ 8,  8, 10, 12, 12, 10,  8,  8, //rook
	8,  8, 10, 12, 12, 10,  8,  8,
	8,  8, 10, 12, 12, 10,  8,  8,
	10, 10, 12, 12, 12, 12, 10, 10,
	16, 16, 16, 16, 16, 16, 16, 16,
	12, 12, 14, 16, 16, 14, 12, 12,
	18, 18, 20, 22, 22, 20, 18, 18,
	10, 10, 12, 14, 14, 12, 10, 10 },

	{ 8,  6,  8,  8,  8,  8,  6,  8, //queen
	8,  8,  8, 10, 10,  8,  8,  8,
	8,  8, 10, 10, 10, 10,  8,  8,
	8, 10, 12, 12, 12, 12, 10,  8,
	10, 12, 12, 14, 14, 12, 12, 10,
	8, 10, 12, 14, 14, 12, 10,  8,
	8, 10, 10, 12, 12, 10, 10,  8,
	10,  8, 10, 10, 10, 10,  8, 10 },

	{ 20, 20,-10,-20,-20, 10, 20, 20, //king
	10, 20,  0,-30,-30,  0, 15, 10,
	0,-10,-20,-40,-40,-20,-10,  0,
	-10,-20,-40,-50,-50,-40,-20,-10,
	-60,-60,-60,-60,-60,-60,-60,-60,
	-80,-80,-80,-80,-80,-80,-80,-80,
	-80,-80,-80,-80,-80,-80,-80,-80,
	-80,-80,-80,-80,-80,-80,-80,-80 }
};

int Engine::LeafEval()
{
	int score[2] = { 0,0 };
	for (int i = 0; i < 2; i++)
	{
		score[i] += 100 * popcnt(CurrentPos.Pieces[i][PIECE_PAWN]);
		score[i] += 300 * popcnt(CurrentPos.Pieces[i][PIECE_KNIGHT]);
		score[i] += 300 * popcnt(CurrentPos.Pieces[i][PIECE_BISHOP]);
		score[i] += 500 * popcnt(CurrentPos.Pieces[i][PIECE_ROOK]);
		score[i] += 900 * popcnt(CurrentPos.Pieces[i][PIECE_QUEEN]);
	}

	for (int i = 0; i < 64; i++)
	{
		int sq = CurrentPos.Squares[i];
		score[getSquare2Color(sq)] += PieceSqValues[getSquare2Piece(sq)][getColorMirror(getSquare2Color(sq), i)];
	}

	int ret = score[0] - score[1];
	if (CurrentPos.Turn == COLOR_BLACK)
	{
		ret = -ret;
	}
	return ret;
}

int Engine::LeafEval_NN()
{
	PosNN.copyFromPosition(CurrentPos);
	for (uint64_t i = 0; i < BatchSize; i++)
	{
		memcpy(&InputTensor(i, 0, 0, 0), PosNN.Squares.mData, sizeof(Float) * 8 * 8 * 14);
	}
	int eval = mNet->get_eval(InputTensor)*100;
	if (CurrentPos.Turn == COLOR_BLACK)
		eval = -eval;
	return eval;
}

void Engine::learn_eval(int num_games)
{
	uint64_t c = 0;
	for (int i = 0; i < num_games; i++)
	{
		printf("Game: %d\n", i + 1);
		CurrentPos.setStartPos();
		while (true)
		{
			std::vector<Move> moves;
			moves.reserve(128);
			CurrentPos.generateMoves(moves);

			Move m = moves[rand() % moves.size()];

			int leaf = LeafEval();

			int r = rand() % 10;
			if (r == 0)
			{
				Data* d = &Database[DBCounter];
				d->pos.copyFromPosition(CurrentPos);
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					leaf = -leaf;
				}
				d->eval = leaf;
				moveToTensor(m, &d->move);

				DBCounter++;
				if (DBCounter == DATABASE_SIZE)
				{
					DBCounter = 0;
				}

				if (DBSize < DATABASE_SIZE)
				{
					DBSize++;
				}
			}

			if (DBSize == DATABASE_SIZE && c%64==0)
			{
				//printf("EPOCH\n");
				for (int epoch = 0; epoch < 100; epoch++)
				{
					Float error = 0;
					for (int batch = 0; batch < 100; batch++)
					{
						for (uint64_t i = 0; i < BatchSize; i++)
						{
							//size_t id = rand() % DBSize;
							size_t id = batch*BatchSize + i;
							memcpy(&InputTensor(i * 8 * 8 * 14), Database[id].pos.Squares.mData, sizeof(Float) * 8 * 8 * 14);
							//memcpy(&OutputTensor(i * 2 * 64), Database[id].move.mData, sizeof(Float) * 2 * 64);
							OutputEvalTensor(i) = Database[id].eval / 100.0;
						}
						for (int run = 0; run < 1; run++)
						{
							error += mNet->train(InputTensor, nullptr, &OutputEvalTensor);
							//printf("Error: %f\n", mNet->train(InputTensor, nullptr, &OutputEvalTensor));
						}
					}
					if (epoch == 9)
					{
						printf("Final error: %f, avg: %f\n", error, error/(64*100));
					}
				}
				
			}

			CurrentPos.makeMove(m);
			if (CurrentPos.getGameStatus() != STATUS_NOTOVER || CurrentPos.movelist.size() > 100)
			{
				break;
			}
			c++;
		}
	}
}

void Engine::learn_eval_NN(int num_games)
{
	uint64_t c = 0;
	for (int i = 0; i < num_games; i++)
	{
		printf("Game: %d\n", i + 1);
		CurrentPos.setStartPos();
		while (true)
		{
			Move m = createNullMove(CurrentPos.EPSquare);
			int eval = 0;

			int r1 = rand() % 100;
			if (r1 < 25)
			{
				std::vector<Move> moves;
				moves.reserve(128);
				CurrentPos.generateMoves(moves);

				m = moves[rand() % moves.size()];
				assert(m.isNullMove() == false);
			}
			else
			{
				Bitset hash = CurrentPos.HashKey;

				GoReturn go = go_alphabeta();
				m = go.m;
				eval = go.eval;
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					eval = -eval;
				}
				assert(m.isNullMove() == false);
				assert(CurrentPos.HashKey == hash);
			}
			//assert(m.isNullMove() == false);

			int r2 = rand() % 1;
			if (r2 == 0 && r1>=25)
			{
				Data* d = &Database[DBCounter];
				d->pos.copyFromPosition(CurrentPos);
				d->eval = eval;
				moveToTensor(m, &d->move);

				DBCounter++;
				if (DBCounter == DATABASE_SIZE)
				{
					DBCounter = 0;
					printf("Counter reset\n");
				}

				if (DBSize < DATABASE_SIZE)
				{
					DBSize++;
				}
			}

			if (DBSize >= BatchSize && c % 64 == 0)
			{
				//printf("EPOCH\n");
				for (int epoch = 0; epoch < 100; epoch++)
				{
					Float error = 0;
					for (int batch = 0; batch < DBSize/BatchSize; batch++)
					{
						for (uint64_t i = 0; i < BatchSize; i++)
						{
							//size_t id = rand() % DBSize;
							size_t id = batch*BatchSize + i;
							memcpy(&InputTensor(i * 8 * 8 * 14), Database[id].pos.Squares.mData, sizeof(Float) * 8 * 8 * 14);
							//memcpy(&OutputTensor(i * 2 * 64), Database[id].move.mData, sizeof(Float) * 2 * 64);
							OutputEvalTensor(i) = Database[id].eval / 100.0;
						}
						for (int run = 0; run < 1; run++)
						{
							error += mNet->train(InputTensor, nullptr, &OutputEvalTensor);
							//printf("Error: %f\n", mNet->train(InputTensor, nullptr, &OutputEvalTensor));
						}
					}
					if (epoch == 9)
					{
						printf("Final error: %f, avg: %f\n", error, error / (BatchSize * DBSize));
					}
				}
			}

			CurrentPos.makeMove(m);
			if (CurrentPos.getGameStatus() != STATUS_NOTOVER || CurrentPos.movelist.size() > 100)
			{
				break;
			}
			c++;
		}
	}
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
