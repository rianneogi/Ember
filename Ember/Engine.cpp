#include "Engine.h"

const int DATABASE_SIZE = 2500;
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

Move Engine::go_alphabeta()
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
		int score = -AlphaBeta(-CONST_INF, CONST_INF, 4);
		CurrentPos.unmakeMove(moves[i]);

		if (score > bestscore)
		{
			bestscore = score;
			bestmove = moves[i];
		}
	}
	return bestmove;
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
	int status = CurrentPos.getGameStatus();
	if (status != STATUS_NOTOVER)
	{
		if (status == STATUS_3FOLDREP || status == STATUS_STALEMATE || status == STATUS_INSUFFICIENTMAT)
		{
			return 0;
		}
	}

	if (depth == 0)
		return LeafEval();

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
			memcpy(&InputTensor(i * 14 * 8 * 8), &Database[id].pos.mTensor.mData, sizeof(Float) * 14 * 8 * 8);
			memcpy(&OutputTensor(i * 2 * 8), &Database[id].move.mData, sizeof(Float) * 2 * 64);
			OutputEvalTensor(i) = Database[id].eval;
		}
		mNet->train(InputTensor, nullptr, &OutputEvalTensor);
	}

	return bestscore;
}

int Engine::LeafEval()
{
	int score[2] = { 0,0 };
	for (int i = 0; i < 2; i++)
	{
		score[i] += 100 * popcnt(CurrentPos.Pieces[i][PIECE_PAWN]);
		score[i] += 320 * popcnt(CurrentPos.Pieces[i][PIECE_KNIGHT]);
		score[i] += 350 * popcnt(CurrentPos.Pieces[i][PIECE_BISHOP]);
		score[i] += 500 * popcnt(CurrentPos.Pieces[i][PIECE_ROOK]);
		score[i] += 950 * popcnt(CurrentPos.Pieces[i][PIECE_QUEEN]);
	}
	int ret = score[0] - score[1];
	if (CurrentPos.Turn == COLOR_BLACK)
	{
		ret = -ret;
	}
	return ret;
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

			Data* d = &Database[DBCounter];
			d->pos.copyFromPosition(CurrentPos);
			d->eval = LeafEval();
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

			if (c % 64 == 0)
			{
				for (uint64_t i = 0; i < BatchSize; i++)
				{
					size_t id = rand() % DBSize;
					memcpy(&InputTensor(i*8*8*14), Database[id].pos.mTensor.mData, sizeof(Float) * 8 * 8 * 14);
					memcpy(&OutputTensor(i * 2 * 64), Database[id].move.mData, sizeof(Float) * 2 * 64);
					OutputEvalTensor(i) = Database[id].eval/100.0;
				}
				//CurrentPos.display(0);
				printf("Error: %f\n", mNet->train(InputTensor, nullptr, &OutputEvalTensor));
				//printf("trained %d\n", CurrentPos.movelist.size());
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

Data::Data() : move(make_shape(20, 2, 64))
{
}

Data::~Data()
{
	move.freemem();
}
