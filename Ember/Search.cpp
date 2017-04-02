#include "Engine.h"

jmp_buf JumpEnv;

int getNPS(int nodes, int milliseconds)
{
	if (milliseconds != 0)
	{
		return (nodes / (milliseconds / 1000.0));
	}
	return 0;
}

Move Engine::go(int mode, int wtime, int btime, int winc, int binc, bool print)
{
	/*Clock timer;
	timer.Start();
	GoReturn go = go_alphabeta(4);
	timer.Stop();
	if (print)
	{
		std::cout << "info score cp " << go.eval << " depth " << 4 << " nodes " << NodeCount <<
			" nps " << getNPS(NodeCount, timer.ElapsedMilliseconds()) << std::endl;
	}
	return go.m;*/

	int MAXDEPTH = 100;

	AllocatedTime = 1;
	uint64_t mytime = 1;
	uint64_t opptime = 1;
	if (mode == MODE_MOVETIME)
	{
		AllocatedTime = wtime;
	}
	else
	{
		if (CurrentPos.Turn == COLOR_WHITE)
		{
			mytime = wtime;
			opptime = btime;
		}
		else
		{
			mytime = btime;
			opptime = wtime;
		}
		AllocatedTime = std::max((uint64_t)1, mytime / 15);
	}
	if (mode == MODE_DEPTH)
	{
		MAXDEPTH = wtime;
	}

	std::cout << "info Allocated time: " << AllocatedTime << std::endl;
	Move bestmove = createNullMove(CurrentPos.EPSquare);

	int initial_move_number = CurrentPos.movelist.size();

	int jmpreturn = setjmp(JumpEnv);
	if (jmpreturn != 0)
	{
		while (CurrentPos.movelist.size() > initial_move_number)
		{
			CurrentPos.takebackMove();
		}
		return bestmove;
	}

	Timer.Reset();
	Timer.Start();
	for (int depth = 1; depth < MAXDEPTH; depth++)
	{
		GoReturn go = go_alphabeta(depth);
		bestmove = go.m;

		std::cout << "info score cp " << go.eval << " depth " << depth << " nodes " << NodeCount <<
			" nps " << getNPS(NodeCount, Timer.ElapsedMilliseconds()) <<
			" pv " << bestmove.toString() << std::endl;
	}
	return bestmove;
}

GoReturn Engine::go_alphabeta(int depth)
{
	NodeCount = 0;
	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);
	int bestscore = -CONST_INF;
	Move bestmove = createNullMove(CurrentPos.EPSquare);
	//assert(popcnt(CurrentPos.Pieces[COLOR_WHITE][PIECE_KING]) != 0);
	//assert(popcnt(CurrentPos.Pieces[COLOR_BLACK][PIECE_KING]) != 0);
	//assert(CurrentPos.underCheck(getOpponent(CurrentPos.Turn)) == false);
	assert(moves.size() > 0);
	for (size_t i = 0; i < moves.size(); i++)
	{
		Move m = getNextMove(moves, i, 0);
		CurrentPos.makeMove(m);
		int score = -AlphaBeta(-CONST_INF, CONST_INF, depth - 1, 1);
		CurrentPos.unmakeMove(m);
		assert(score >= -CONST_INF && score <= CONST_INF);
		if (score >= bestscore)
		{
			bestscore = score;
			bestmove = m;
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
		int score = -Negamax(depth - 1, 1);
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
	NodeCount++;
	if (NodeCount % 1028 == 0)
	{
		checkup();
	}

	if (depth == 0)
	{
#ifdef TRAINING_BUILD
		return LeafEval_NN();
#else
		return LeafEval_NN();
#endif
	}


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
		int score = -AlphaBeta(-beta, -alpha, depth - 1, ply + 1);
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
		int score = -Negamax(depth - 1, ply + 1);
		CurrentPos.unmakeMove(moves[i]);

		if (score > bestscore)
		{
			bestscore = score;
			bestmove = moves[i];
		}
	}

	if (bestscore != leafeval && depth >= 4)
	{
		Data* d = &Database[DBCounter];
		d->pos.copyFromPosition(CurrentPos);
		d->eval = leafeval;
		moveToTensor(bestmove, &d->move);

		DBCounter++;
		if (DBCounter == DATABASE_MAX_SIZE)
		{
			DBCounter = 0;
		}

		if (DBSize < DATABASE_MAX_SIZE)
		{
			DBSize++;
		}

		for (uint64_t i = 0; i < BatchSize; i++)
		{
			size_t id = rand() % DBSize;
			memcpy(&InputTensor(i * 14 * 8 * 8), &Database[id].pos.Squares.mData, sizeof(Float) * 14 * 8 * 8);
			memcpy(&OutputMoveTensor(i * 2 * 8), &Database[id].move.mData, sizeof(Float) * 2 * 64);
			OutputEvalTensor(i) = Database[id].eval;
		}
		NetTrain->train(InputTensor, nullptr, &OutputEvalTensor);
	}
	return bestscore;
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

	auto key = CurrentPos.HashKey;
	auto os = CurrentPos.OccupiedSq;

	for (unsigned int i = 0; i < vec.size(); i++)
	{
		Move m = vec[i];
		CurrentPos.makeMove(m);
		count += perft(depth - 1);
		CurrentPos.unmakeMove(m);
	}

	assert(key == CurrentPos.HashKey);
	assert(os == CurrentPos.OccupiedSq);

	return count;
}

void Engine::checkup()
{
	Timer.Stop();
	if (Timer.ElapsedMilliseconds() >= AllocatedTime)
	{
		longjmp(JumpEnv, Timer.ElapsedMilliseconds());
	}
}
