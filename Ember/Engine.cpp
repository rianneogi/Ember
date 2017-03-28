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

	for (int i = 0; i<2; i++)
	{
		for (int j = 0; j<100; j++)
		{
			KillerMoves[i][j] = createNullMove(CurrentPos.EPSquare);
		}
	}
	for (int i = 0; i<64; i++)
	{
		for (int j = 0; j<64; j++)
		{
			HistoryScores[i][j] = 0;
		}
	}

	Table = new TranspositionTable(4096);
}

Engine::~Engine()
{
	delete[] Database;
	delete mNet;
	delete Table;
	Table = NULL;
	Database = NULL;
	mNet = NULL;
}

Move Engine::go(int mode, int wtime, int btime, int winc, int binc, bool print)
{
	GoReturn go = go_alphabeta(4);
	if (print)
	{
		std::cout << "info score cp " << go.eval << std::endl;
	}
	return go.m;
}

GoReturn Engine::go_alphabeta(int depth)
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
		int score = -AlphaBeta(-CONST_INF, CONST_INF, depth-1,1);
		CurrentPos.unmakeMove(moves[i]);

		if (score >= bestscore)
		{
			bestscore = score;
			bestmove = moves[i];
		}
	}
	return GoReturn(bestmove, bestscore);
}

Move Engine::go_negamax(int depth)
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
		int score = -Negamax(depth-1,1);
		CurrentPos.unmakeMove(moves[i]);

		if (score > bestscore)
		{
			bestscore = score;
			bestmove = moves[i];
		}
	}
	return bestmove;
}

int Engine::AlphaBeta(int alpha, int beta, int depth, int ply)
{
	if (depth == 0)
		return LeafEval_NN();

	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);

	//assert(popcnt(CurrentPos.Pieces[COLOR_WHITE][PIECE_KING]) != 0);
	//assert(popcnt(CurrentPos.Pieces[COLOR_BLACK][PIECE_KING]) != 0);
	//assert(CurrentPos.underCheck(getOpponent(CurrentPos.Turn)) == false);

	int probe = Table->Probe(CurrentPos.HashKey, depth, alpha, beta);
	if (probe != CONS_TTUNKNOWN)
	{
		if (ply != 0)
		{
			return probe;
		}
	}

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
	int bound = TT_ALPHA;
	Move bestmove = CONST_NULLMOVE;
	for (int i = 0; i < moves.size(); i++)
	{
		Move m = getNextMove(moves, i, ply);

		CurrentPos.makeMove(m);
		int score = -AlphaBeta(-beta, -alpha, depth - 1, ply+1);
		CurrentPos.unmakeMove(m);

		if (score >= beta)
		{
			if (noMaterialGain(m))
			{
				//if(Table.getBestMove(pos.TTKey)!=m) //dont store hash move as a killer
				setKiller(m, ply);

				int bonus = depth*depth;
				HistoryScores[m.getMovingPiece()][m.getTo()] += bonus;
				if (HistoryScores[m.getMovingPiece()][m.getTo()] > 200000) //prevent overflow of history values
				{
					for (int i = 0; i < 6; i++)
					{
						for (int j = 0; j < 64; j++)
						{
							HistoryScores[i][j] /= 2;
						}
					}
				}
			}
			Table->Save(CurrentPos.HashKey, depth, bestscore, TT_BETA, m); //not storing best move for now
			return score;
		}
		else if (score > bestscore)
		{
			if (score > alpha)
			{
				bound = TT_EXACT;
				alpha = score;
			}
			bestscore = score;
			bestmove = m;
		}
	}
	Table->Save(CurrentPos.HashKey, depth, bestscore, bound, bestmove); //not storing best move for now
	return bestscore;
}

int Engine::Negamax(int depth, int ply)
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
		int score = -Negamax(depth - 1, ply+1);
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
					if (epoch == 99)
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
			if (r1 < 75)
			{
				std::vector<Move> moves;
				moves.reserve(128);
				CurrentPos.generateMoves(moves);

				m = moves[rand() % moves.size()];
				assert(m.isNullMove() == false);

				eval = LeafEval();
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					eval = -eval;
				}
			}
			else
			{
				Bitset hash = CurrentPos.HashKey;

				GoReturn go = go_alphabeta(4);
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

			int r2 = rand() % 3;
			if (r2 == 0)
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

			if (DBSize >= DATABASE_SIZE && c % 64 == 0)
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
					if (epoch == 99)
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
