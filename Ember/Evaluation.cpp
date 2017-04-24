#include "Engine.h"

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

int Engine::LeafEval_MatOnly()
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

	int ret = score[0] - score[1];
	if (CurrentPos.Turn == COLOR_BLACK)
	{
		ret = -ret;
	}
	return ret;
}

Float Engine::LeafEval_NN()
{
	PosNN.copyFromPosition(CurrentPos);
	
	//for (uint64_t i = 0; i < BatchSize; i++)
	//{
	//	memcpy(&InputTensor(i, 0, 0, 0), PosNN.Squares.mData, sizeof(Float) * 8 * 8 * 14);
	//}
	
	Float eval = EvalNet_Play->get_eval(PosNN.Squares) * 100;
	if (CurrentPos.Turn == COLOR_BLACK)
		eval = -eval;
	
	return eval;
}

bool Engine::isDraw()
{
	Bitset ColorPieces[2];
	long long ColorPiecesCount[2];

	if (popcnt(CurrentPos.OccupiedSq) > 3)
	{
		return false;
	}

	for (int i = 0;i < 2;i++)
	{
		ColorPieces[i] = CurrentPos.Pieces[i][PIECE_PAWN] | CurrentPos.Pieces[i][PIECE_KNIGHT] |
			CurrentPos.Pieces[i][PIECE_BISHOP] | CurrentPos.Pieces[i][PIECE_ROOK] |
			CurrentPos.Pieces[i][PIECE_QUEEN] | CurrentPos.Pieces[i][PIECE_KING];
		ColorPiecesCount[i] = popcnt(ColorPieces[i]);
	}
	if (ColorPiecesCount[COLOR_BLACK] == 1 && ColorPiecesCount[COLOR_WHITE] == 2
		&& (popcnt(CurrentPos.Pieces[COLOR_WHITE][PIECE_KNIGHT]) == 1 || popcnt(CurrentPos.Pieces[COLOR_WHITE][PIECE_BISHOP]) == 1))
	{
		return true;
	}
	if (ColorPiecesCount[COLOR_WHITE] == 1 && ColorPiecesCount[COLOR_BLACK] == 2
		&& (popcnt(CurrentPos.Pieces[COLOR_BLACK][PIECE_KNIGHT]) == 1 || popcnt(CurrentPos.Pieces[COLOR_BLACK][PIECE_BISHOP]) == 1))
	{
		return true;
	}
	if (ColorPiecesCount[COLOR_WHITE] == 1 && ColorPiecesCount[COLOR_BLACK] == 1)
	{
		return true;
	}
	return false;
}