#include "Engine.h"

int PieceMaterial[7] = { 100,300,300,500,900,0,0 };

int Engine::QSearch(int alpha, int beta)
{
	//quisctime.Start();

	if (isDraw()) return 0;

	NodeCount++;
	if (NodeCount %1024 == 0)
	{
		checkup();
		//nodes = 0;
	}
	/*if(pos.getGameStatus()!=STATUS_NOTOVER)
	{
	int val = LeafEval(alpha,beta);
	if(val >= beta)
	return beta;
	else if(val < alpha)
	return alpha;
	return val;
	}*/
	int stand_pat = 0;
	//ProbeStruct probe = Table.Probe(pos.TTKey, -1, alpha, beta);
	//if (probe.found && probe.entry->bound == TT_EXACT)
	//{
	//	return probe.score;
	//	stand_pat = probe.score; //use TT probe as leafeval
	//}
	//else
	//{
#ifdef TRAINING_BUILD
	stand_pat = LeafEval_MatOnly();

	if (rand() % 20 == 0)
	{
		Data* d = &Database[DBCounter];
		d->pos.copyFromPosition(CurrentPos);
		d->eval = stand_pat / 100.0;
		if (CurrentPos.Turn == COLOR_BLACK)
			d->eval = -d->eval;
		//moveToTensor(m, &d->move);

		Position p;
		d->pos.copyToPosition(p);
		for (int i = 0; i < 64; i++)
		{
			assert(CurrentPos.Squares[i] == p.Squares[i]);
		}

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
			for (uint64_t i = 0; i < BatchSize; i++)
			{
				size_t id = rand() % DBSize;
				//Database[id].pos.Squares.print_raw();
				memcpy(&InputTensor(i * 14 * 8 * 8), Database[id].pos.Squares.mData, sizeof(Float) * 14 * 8 * 8);
				//memcpy(&OutputMoveTensor(i * 2 * 8), &Database[id].move.mData, sizeof(Float) * 2 * 64);
				OutputEvalTensor(i) = Database[id].eval;
			}
			Float error = NetTrain->train(InputTensor, &OutputEvalTensor, nullptr);
			printf("Error: %f, Avg: %f\n", error, error / BatchSize);
		}
	}
#else
	stand_pat = LeafEval();
#endif
	//} 
	if (stand_pat >= beta) //standpat
	{
		return stand_pat;
	}
	if (stand_pat <= alpha - PieceMaterial[PIECE_QUEEN]) //big delta pruning
	{
		return stand_pat;
	}
	if (alpha < stand_pat)
	{
		alpha = stand_pat;
	}
	Move m;
	int score = 0;

	std::vector<Move> vec; //generate moves
	vec.reserve(128);
	//movegentime.Start();
	CurrentPos.generateCaptures(vec);
	//pos.generatePawnPushes(vec);
	//movegentime.Stop();

	//vector<int> scores; //generate move scores
	//scores.reserve(128);
	//generateCaptureScores(vec, scores);

	//int material = getBoardMaterial<COLOR_WHITE>() + getBoardMaterial<COLOR_BLACK>();
	Move bestmove = CONST_NULLMOVE;
	int bound = TT_ALPHA;
	for (int i = 0;i<vec.size();i++)
	{
		//m = getHighestScoringMove(vec,i);
		m = vec[i];
		int special = m.getSpecial();
		int captured = m.getCapturedPiece();
		if ((stand_pat + PieceMaterial[getSquare2Piece(captured)] + 200 < alpha) //delta pruning
			&& (special != PIECE_QUEEN && special != PIECE_KNIGHT  && special != PIECE_ROOK && special != PIECE_BISHOP) //not a promotion
																														//&& (material > EndgameMaterial)
			)
		{
			continue;
		}
		if (StaticExchangeEvaluation(m.getTo(), m.getFrom(), m.getMovingPiece(), captured) < 0)
			continue;
		//break; //since moves are sorted by SEE, we know remaining moves will also have SEE<0
		//if (getSquare2Piece(m.getCapturedPiece()) == PIECE_KING) //captured opponent king
		//	return CONS_INF;
		if (!CurrentPos.tryMove(m))
		{
			continue;
		}
		//pos.forceMove(m);
		/*ply++;
		if (ply > SelectiveDepth)
		{
			SelectiveDepth = ply;
		}*/
		score = -QSearch(-beta, -alpha);
		CurrentPos.unmakeMove(m);
		//ply--;
		if (score >= beta)
		{
			//Table.Save(pos.TTKey, -1, score, TT_BETA, m);
			return score;
		}

		if (alpha < score)
		{
			alpha = score;
			bestmove = m;
			bound = TT_EXACT;
		}

	}
	//Table.Save(pos.TTKey, -1, alpha, TT_ALPHA, bestmove);
	//quisctime.Stop();
	return alpha;
}

int Engine::StaticExchangeEvaluation(int to, int from, int movpiece, int capt)
{
	capt = getSquare2Piece(capt);
	/*if (PieceMaterial[capt] >= PieceMaterial[movpiece])
	{
	return PieceMaterial[capt] - PieceMaterial[movpiece];
	}*/
	int gain[100], d = 0;
	Bitset occ = CurrentPos.OccupiedSq;
	/*Bitset occ90 = pos.OccupiedSq90;
	Bitset occ45 = pos.OccupiedSq45;
	Bitset occ135 = pos.OccupiedSq135;*/
	Bitset pieces[2][6];
	for (int i = 0;i<2;i++)
	{
		for (int j = 0;j<6;j++)
		{
			pieces[i][j] = CurrentPos.Pieces[i][j];
		}
	}
	int turn = CurrentPos.Turn;
	gain[d] = PieceMaterial[capt];
	//cout << "gain " << d << " is " << gain[0] << endl;
	Move m = CONST_NULLMOVE;
	do
	{
		//cout << m.toString() << movpiece << endl;
		d++; // next depth and side
		gain[d] = PieceMaterial[movpiece] - gain[d - 1]; // speculative store, if defended
		if (std::max(-gain[d - 1], gain[d]) < 0) break; // pruning does not influence the result
												   //cout << "gain " << d << " is " << gain[d] << endl;

		CurrentPos.OccupiedSq ^= getPos2Bit(from); // reset bit in temporary occupancy (for x-Rays)
											/*pos.OccupiedSq90 ^= getPos2Bit(getturn90(from));
											pos.OccupiedSq45 ^= getPos2Bit(getturn45(from));
											pos.OccupiedSq135 ^= getPos2Bit(getturn135(from));*/
		CurrentPos.Pieces[turn][movpiece] ^= getPos2Bit(from);
		turn = getOpponent(turn);

		m = CurrentPos.getSmallestAttacker(turn, to);
		from = m.getFrom();
		capt = movpiece;
		movpiece = m.getMovingPiece();
	} while (m.isNullMove() == false);

	CurrentPos.OccupiedSq = occ;
	/*pos.OccupiedSq90 = occ90;
	pos.OccupiedSq45 = occ45;
	pos.OccupiedSq135 = occ135;*/
	for (int i = 0;i<2;i++)
	{
		for (int j = 0;j<6;j++)
		{
			CurrentPos.Pieces[i][j] = pieces[i][j];
		}
	}
	//cout << "gain " << d << " is " << gain[d] << endl;
	while (--d)
	{
		gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
		//d--;
		//cout << "gain " << d << " is " << gain[d] << endl;
	}
	return gain[0];
}