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

SearchResult Engine::go(int mode, int wtime, int btime, int winc, int binc, bool print)
{
	NodeCount = 0;
	BetaCutoffCount = 0;
	BetaCutoffValue = 0;

	int MAXDEPTH = 100;
	TimeMode = mode;
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

	if(print)
		std::cout << "info string Allocated time: " << AllocatedTime << std::endl;
	SearchResult best(CONST_NULLMOVE, -1);

	int initial_move_number = CurrentPos.movelist.size();

	int jmpreturn = setjmp(JumpEnv);
	if (jmpreturn != 0)
	{
		while (CurrentPos.movelist.size() > initial_move_number)
		{
			CurrentPos.takebackMove();
		}
		return best;
	}

	Timer.Reset();
	Timer.Start();
	for (int depth = 1; depth < MAXDEPTH; depth++)
	{
		best = go_alphabeta(depth);

		if(print)
			std::cout << "info score cp " << best.eval << " depth " << depth << " nodes " << NodeCount <<
				" nps " << getNPS(NodeCount, Timer.ElapsedMilliseconds()) <<
				" pv " << best.m.toString() << std::endl;
		//std::cout << "info string Betacuff ratio: " << ((BetaCutoffCount*1.0) / NodeCount) << std::endl;
		//std::cout << "info string Betacuff movecount: " << ((BetaCutoffValue*1.0) / BetaCutoffCount) << std::endl;

		assert(best.eval >= -CONST_INF && best.eval <= CONST_INF);
		assert(best.m.isNullMove() == false);
	}
	return best;
}

SearchResult Engine::go_alphabeta(int depth)
{
	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);
	int bestscore = -CONST_INF;
	Move bestmove = createNullMove(CurrentPos.EPSquare);
	//assert(popcnt(CurrentPos.Pieces[COLOR_WHITE][PIECE_KING]) != 0);
	//assert(popcnt(CurrentPos.Pieces[COLOR_BLACK][PIECE_KING]) != 0);
	//assert(CurrentPos.underCheck(getOpponent(CurrentPos.Turn)) == false);
	bool found_legal = false;
	for (size_t i = 0; i < moves.size(); i++)
	{
		Move m = getNextMove(moves, i, 0);
		if (!CurrentPos.tryMove(m))
		{
			continue;
		}
		found_legal = true;
		int score = -AlphaBeta(-CONST_INF, CONST_INF, depth - 1, 1);
		CurrentPos.unmakeMove(m);
		assert(score >= -CONST_INF && score <= CONST_INF);
		if (score >= bestscore)
		{
			bestscore = score;
			bestmove = m;
		}
	}
	assert(found_legal);
	assert(bestmove.isNullMove() == false);
	return SearchResult(bestmove, bestscore);
}

int Engine::AlphaBeta(int alpha, int beta, int depth, int ply)
{
	NodeCount++;
	if (NodeCount % 1028 == 0)
	{
		if(TimeMode==MODE_MOVETIME || TimeMode==MODE_DEFAULT)
			checkup();
	}
	
	if (depth == 0)
	{
#ifdef TRAINING_BUILD
		return QSearch(alpha, beta);
#else
		return QSearch(alpha, beta);
#endif
	}
	
	/*assert(popcnt(CurrentPos.Pieces[COLOR_WHITE][PIECE_KING]) != 0);
	assert(popcnt(CurrentPos.Pieces[COLOR_BLACK][PIECE_KING]) != 0);
	assert(CurrentPos.underCheck(getOpponent(CurrentPos.Turn)) == false);*/
	
	int probe = Table->Probe(CurrentPos.HashKey, depth, alpha, beta);
	if (probe != CONS_TTUNKNOWN)
	{
		if (ply != 0)
		{
			return probe;
		}
	}

	/*PositionNN pnn(CurrentPos);
	Position pos;
	pnn.copyToPosition(pos);
	for (int i = 0; i < 64; i++)
	{
		assert(pos.Squares[i] == CurrentPos.Squares[i]);
	}*/

#ifdef DO_NULL_MOVE
	Bitset Pieces = CurrentPos.OccupiedSq ^ CurrentPos.Pieces[COLOR_WHITE][PIECE_PAWN] ^ CurrentPos.Pieces[COLOR_BLACK][PIECE_PAWN];
	int pieceCount = popcnt(Pieces);
	if (depth >= 3 && ply >= 0 && CurrentPos.underCheck(CurrentPos.Turn) == false 
		&& CurrentPos.movelist[CurrentPos.movelist.size()-1].isNullMove() == false
		&& (pieceCount>2) //side to move does not have only pawns(to avoid zugzwang)
						  //&& Evaluation[ply] >= beta
		)
	{
		int R = depth > 5 ? 3 : 2; //dynamic depth-based reduction
		//int R = ((823 + 67 * depth) / 256 + std::min(max(0, Evaluation[ply] - beta) / PieceMaterial[PIECE_PAWN], 3));
		Move m = createNullMove(CurrentPos.EPSquare);

		CurrentPos.makeMove(m);
		int score = -AlphaBeta(-beta, -beta + 1, std::max(0, depth - R), ply+1); //make a null-window search (we don't care by how much it fails high, if it does)
		CurrentPos.unmakeMove(m);

		if (score >= beta)
		{
			//cout << "Null move cutoff " << beta << endl;
			/*if (probe.avoidnull)
			badavoidnull++;*/
			return score;
		}
	}
#endif

	std::vector<Move> moves;
	moves.reserve(128);
	CurrentPos.generateMoves(moves);

	std::vector<Move> oldmoves;
	oldmoves.reserve(128);
	
	int bestscore = -CONST_INF;
	int bound = TT_ALPHA;
	Move bestmove = CONST_NULLMOVE;
	bool found_legal = false;
	for (int i = 0; i < moves.size(); i++)
	{
#ifdef TRAINING_BUILD
		Move m = getNextMove(moves, i, ply);
#else
		Move m = getNextMove_NN(moves, i, ply);
#endif
		
		if (!CurrentPos.tryMove(m))
		{
			continue;
		}
		
		found_legal = true;
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
			Table->Save(CurrentPos.HashKey, depth, bestscore, TT_BETA, m);
			bestmove = m;

			if (rand() % 512 == 0)
			{
				for (int i = 0; i < oldmoves.size(); i++)
				{
					if (oldmoves[i] != bestmove)
					{
						Data* d = &Database[DBCounter];
						d->pos.copyFromPosition(CurrentPos);
						//d->eval = bestscore / 100.0;
						//if (CurrentPos.Turn == COLOR_BLACK)
						//	d->eval = -d->eval;
						d->eval = 0.0;
						moveToTensorPtr(oldmoves[i], d->move.mData);

						DBCounter++;
						if (DBCounter == DATABASE_MAX_SIZE)
						{
							DBCounter = 0;
							printf("DB reset\n");
						}

						if (DBSize < DATABASE_MAX_SIZE)
						{
							DBSize++;
						}
					}
				}
				Data* d = &Database[DBCounter];
				d->pos.copyFromPosition(CurrentPos);
				//d->eval = bestscore / 100.0;
				//if (CurrentPos.Turn == COLOR_BLACK)
				//	d->eval = -d->eval;
				d->eval = 1.0;
				moveToTensorPtr(bestmove, d->move.mData);

				DBCounter++;
				if (DBCounter == DATABASE_MAX_SIZE)
				{
					DBCounter = 0;
					printf("DB reset\n");
				}

				if (DBSize < DATABASE_MAX_SIZE)
				{
					DBSize++;
				}

				if (DBSize >= DATABASE_MAX_SIZE)
				{
					for (int epoch = 0; epoch < 10; epoch++)
					{
						for (uint64_t i = 0; i < BatchSize; i++)
						{
							size_t id = rand() % DBSize;
							//Database[id].pos.Squares.print_raw();
							memcpy(&PositionTensor(i * POSITION_TENSOR_SIZE), Database[id].pos.Squares.mData, sizeof(Float) * POSITION_TENSOR_SIZE);
							memcpy(&MoveTensor(i * MOVE_TENSOR_SIZE), &Database[id].move.mData, sizeof(Float) * MOVE_TENSOR_SIZE);
							SortTensor(i) = Database[id].eval;
						}
						Float error = NetTrain->train(&PositionTensor, &MoveTensor, nullptr, &SortTensor);
						CumulativeSum += error;
						CumulativeCount++;
						if (CumulativeCount % 16 == 0)
							printf("Error: %f, Avg: %f, Cumulative Avg: %f\n", error, error / BatchSize, CumulativeSum / CumulativeCount);
					}
				}
			}

			/*moveToTensorPtr(m, &MoveTensor(SortNetCount, 0));
			SortTensor(SortNetCount, 0) = depth;

			SortNetCount++;
			if (SortNetCount >= BatchSize)
			{
				NetSort->train(MoveTensor, SortTensor);
				SortNetCount = 0;
			}

			for (int j = 0; j < oldmoves.size(); j++)
			{
				moveToTensorPtr(m, &MoveTensor(SortNetCount, 0));
				SortTensor(SortNetCount, 0) = 0.0;

				SortNetCount++;
				if (SortNetCount >= BatchSize)
				{
					NetSort->train(MoveTensor, SortTensor);
					SortNetCount = 0;
				}
			}*/

			BetaCutoffCount++;
			BetaCutoffValue += (i + 1);
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

		oldmoves.push_back(m);
	}
	
	if (!found_legal)
	{
		if (CurrentPos.underCheck(CurrentPos.Turn))
		{
			return -CONST_INF;
		}
		return 0;
	}

	if (bestmove.isNullMove() == false)
	{
		if (rand() % 512 == 0)
		{
			for (int i = 0; i < oldmoves.size(); i++)
			{
				if (oldmoves[i] != bestmove)
				{
					Data* d = &Database[DBCounter];
					d->pos.copyFromPosition(CurrentPos);
					//d->eval = bestscore / 100.0;
					//if (CurrentPos.Turn == COLOR_BLACK)
					//	d->eval = -d->eval;
					d->eval = 0.0;
					moveToTensorPtr(oldmoves[i], d->move.mData);

					DBCounter++;
					if (DBCounter == DATABASE_MAX_SIZE)
					{
						DBCounter = 0;
						printf("DB reset\n");
					}

					if (DBSize < DATABASE_MAX_SIZE)
					{
						DBSize++;
					}
				}
			}
			Data* d = &Database[DBCounter];
			d->pos.copyFromPosition(CurrentPos);
			//d->eval = bestscore / 100.0;
			//if (CurrentPos.Turn == COLOR_BLACK)
			//	d->eval = -d->eval;
			d->eval = 1.0;
			moveToTensorPtr(bestmove, d->move.mData);

			DBCounter++;
			if (DBCounter == DATABASE_MAX_SIZE)
			{
				DBCounter = 0;
				printf("DB reset\n");
			}

			if (DBSize < DATABASE_MAX_SIZE)
			{
				DBSize++;
			}

			/*Position p;
			d->pos.copyToPosition(p);
			for (int i = 0; i < 64; i++)
			{
				assert(CurrentPos.Squares[i] == p.Squares[i]);
			}*/

			if (DBSize >= DATABASE_MAX_SIZE)
			{
				for (int epoch = 0; epoch < 10; epoch++)
				{
					for (uint64_t i = 0; i < BatchSize; i++)
					{
						size_t id = rand() % DBSize;
						//Database[id].pos.Squares.print_raw();
						memcpy(&PositionTensor(i * POSITION_TENSOR_SIZE), Database[id].pos.Squares.mData, sizeof(Float) * POSITION_TENSOR_SIZE);
						memcpy(&MoveTensor(i * MOVE_TENSOR_SIZE), &Database[id].move.mData, sizeof(Float) * MOVE_TENSOR_SIZE);
						SortTensor(i) = Database[id].eval;
					}
					Float error = NetTrain->train(&PositionTensor, &MoveTensor, nullptr, &SortTensor);
					CumulativeSum += error;
					CumulativeCount++;
					if (CumulativeCount % 16 == 0)
						printf("Error: %f, Avg: %f, Cumulative Avg: %f\n", error, error / BatchSize, CumulativeSum / CumulativeCount);
				}
			}
		}
	}

	Table->Save(CurrentPos.HashKey, depth, bestscore, bound, bestmove); 
	return bestscore;
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
			memcpy(&PositionTensor(i * 14 * 8 * 8), &Database[id].pos.Squares.mData, sizeof(Float) * 14 * 8 * 8);
			memcpy(&OutputMoveTensor(i * 2 * 8), &Database[id].move.mData, sizeof(Float) * 2 * 64);
			OutputEvalTensor(i) = Database[id].eval;
		}
		NetTrain->train(&PositionTensor, nullptr, &OutputEvalTensor, nullptr);
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
		if (!CurrentPos.tryMove(m))
		{
			continue;
		}
		count += perft(depth - 1);
		CurrentPos.unmakeMove(m);
	}

	assert(key == CurrentPos.HashKey);
	assert(os == CurrentPos.OccupiedSq);

	return count;
}
