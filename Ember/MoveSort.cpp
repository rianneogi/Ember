#include "Engine.h"

int Engine::getMoveScore(const Move& m, int ply)
{
	int from = m.getFrom();
	int to = m.getTo();
	int capturedpiece = m.getCapturedPiece();
	int special = m.getSpecial();
	int movingpiece = m.getMovingPiece();
	long long score = 1000000;
	/*if (ply < PrincipalVariation.size())
	{
		if (m == PrincipalVariation.at(PrincipalVariation.size() - 1 - ply))
		{
			score += 6000000;
			SortPhase = SORTPHASE_PV;
			cout << "info string pv hit " << ply << " " << m.toString() << " " << (PrincipalVariation.size() - 1 - ply) << endl;
			return score;
		}
	}*/
	if (m == Table->getBestMove(CurrentPos.HashKey)) //history best move is always first, give it a big advantage of 400000
	{
		score += 4000000;
		//SortPhase = SORTPHASE_HASH;
		//tthitcount++;
		return score;
	}
	/*if (!pos.makeMove(m))
	return -10000;
	bool incheck = pos.underCheck(pos.turn);
	pos.unmakeMove(m);
	if (incheck)
	{
	score += 3500000;
	return score;
	}*/
	/*if (incheck[ply] && movingpiece == PIECE_KING)
	{
	SortPhase = SORTPHASE_GOODCAP;
	score += 3200000;
	return score;
	}*/
	if (m.getSpecial() == PIECE_QUEEN) //queen promotion
	{
		//SortPhase = SORTPHASE_GOODCAP;
		score += 3100000;
		return score;
	}
	if (capturedpiece != SQUARE_EMPTY) //a capture
	{
		//if (Threats[ply].getFrom() == to && Threats[ply]!=CONS_NULLMOVE) //capturing a threatening piece
		//{
		//	score += 200000; 
		//}
		//int cap = getSquare2Piece(capturedpiece);
		//int x = StaticExchangeEvaluation(to, from, movingpiece, capturedpiece);
		int x = 0;
		if (x >= 0) //if it is a good capture
		{
			//SortPhase = SORTPHASE_GOODCAP;
			score += 3000000 + x;
			return score;
		}
		else //bad capture
		{
			//SortPhase = SORTPHASE_BADCAP;
			score += -500000 + x;
			return score;
		}
	}
	else if (special == PIECE_PAWN) //enpassant are also captures
	{
		//int x = StaticExchangeEvaluation(to, from, movingpiece, capturedpiece);
		int x = 0;
		if (x >= 0)
		{
			//SortPhase = SORTPHASE_GOODCAP;
			score += 3000000 + x;
			return score;
		}
		else
		{
			//SortPhase = SORTPHASE_BADCAP;
			score += -500000 + x;
			return score;
		}
	}
	else
	{
		//SortPhase = SORTPHASE_KILLER;
		if (from == KillerMoves[0][ply].getFrom() && to == KillerMoves[0][ply].getTo()) //if its a killer move
		{
			score += 2500000;
			return score;
		}
		else if (from == KillerMoves[1][ply].getFrom() && to == KillerMoves[1][ply].getTo())
		{
			score += 2000000;
			return score;
		}
		/*else if (from == KillerMoves[2][ply].getFrom() && to == KillerMoves[2][ply].getTo())
		{
		score += 1500000;
		return score;
		}*/
		//if (ply > 1)
		//{
		//	Move prev = currentVariation[ply - 1];
		//	//if (from == CounterMoves[prev.getMovingPiece()][prev.getTo()][0].getFrom() && to == CounterMoves[prev.getMovingPiece()][prev.getTo()][0].getTo())
		//	if (m == CounterMoves[prev.getMovingPiece()][prev.getTo()][0])
		//	{
		//		score += 1900000;
		//		return score;
		//	}
		//	//else if (from == CounterMoves[prev.getMovingPiece()][prev.getTo()][1].getFrom() && to == CounterMoves[prev.getMovingPiece()][prev.getTo()][1].getTo())
		//	else if (m == CounterMoves[prev.getMovingPiece()][prev.getTo()][1])
		//	{
		//		score += 1800000;
		//		return score;
		//	}
		//}

		//if (ply > 2)
		//{
		//	Move prev = currentVariation[ply - 2];
		//	if (from == FollowupMoves[prev.getMovingPiece()][prev.getTo()][0].getFrom() && to == FollowupMoves[prev.getMovingPiece()][prev.getTo()][0].getTo())
		//		//if (m == FollowupMoves[prev.getMovingPiece()][prev.getTo()][0])
		//	{
		//		score += 1700000;
		//		return score;
		//	}
		//	else if (from == FollowupMoves[prev.getMovingPiece()][prev.getTo()][1].getFrom() && to == FollowupMoves[prev.getMovingPiece()][prev.getTo()][1].getTo())
		//		//else if (m == FollowupMoves[prev.getMovingPiece()][prev.getTo()][1])
		//	{
		//		score += 1600000;
		//		return score;
		//	}
		//}

		{
			//SortPhase = SORTPHASE_HISTORY;

			score += HistoryScores[movingpiece][to]; //sort the rest by history
			return std::max(score, (long long)0);
		}
	}
	std::cout << "info string Move sort error" << std::endl;
	return score;
}

Move Engine::getNextMove(std::vector<Move>& moves, int current_move, int ply)
{
	int bigmoveid = current_move;
	Move bigmove = moves.at(current_move);
	long long bigscore = getMoveScore(bigmove, ply);
	long long x;
	for (int i = current_move + 1; i<moves.size(); i++)
	{
		x = getMoveScore(moves.at(i), ply);
		if (x >= 5000000) //pv or hash move found
		{
			bigscore = x;
			bigmoveid = i;
			bigmove = moves.at(i);
			break;
		}
		if (x>bigscore)
		{
			bigscore = x;
			bigmoveid = i;
			bigmove = moves.at(i);
		}
	}
	Move m = bigmove; //swap move
	moves.at(bigmoveid) = moves.at(current_move);
	moves.at(current_move) = m;
	return m;
}

void Engine::setKiller(const Move& m, int ply)
{
	if (m != KillerMoves[0][ply])
	{
		KillerMoves[1][ply] = KillerMoves[0][ply];
		KillerMoves[0][ply] = m;
	}
}

Move Engine::getNextMove_NN(std::vector<Move>& moves, int current_move, int ply)
{
	int bigmoveid = current_move;
	Move bigmove = moves.at(current_move);
	long long bigscore = getMoveScore(bigmove, ply);
	long long x;
	for (int i = current_move + 1; i<moves.size(); i++)
	{
		//x = getMoveScore(moves.at(i), ply);
		
		moveToTensorPtr(moves[i], &MoveTensor(i, 0));
		//NetSort->mBoard->forward(MoveTensor);
		//x = NetSort->Output->Data(0);

		////if (x >= 5000000) //pv or hash move found
		////{
		////	bigscore = x;
		////	bigmoveid = i;
		////	bigmove = moves.at(i);
		////	break;
		////}
		//if (x>bigscore)
		//{
		//	bigscore = x;
		//	bigmoveid = i;
		//	bigmove = moves.at(i);
		//}
	}
	NetSort->mBoard->forward(MoveTensor);
	for (int i = 0; i < moves.size() - current_move - 1; i++)
	{
		x = NetSort->Output->Data(i);
		if (x>bigscore)
		{
			bigscore = x;
			bigmoveid = i;
			bigmove = moves.at(i);
		}
	}
	Move m = bigmove; //swap move
	moves.at(bigmoveid) = moves.at(current_move);
	moves.at(current_move) = m;
	return m;
}
