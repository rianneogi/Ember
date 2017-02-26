#include "Bitops.h"

int Opponent[2] = { COLOR_BLACK,COLOR_WHITE };

int Square2Piece[13] = { PIECE_NONE,PIECE_PAWN,PIECE_KNIGHT,PIECE_BISHOP,PIECE_ROOK,PIECE_QUEEN,
PIECE_KING,PIECE_PAWN,PIECE_KNIGHT,PIECE_BISHOP,PIECE_ROOK,PIECE_QUEEN,PIECE_KING };
int Square2Color[13] = { COLOR_NONE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,COLOR_WHITE,
COLOR_WHITE,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK,COLOR_BLACK };
int Piece2Square[2][7] = { { SQUARE_WHITEPAWN,SQUARE_WHITEKNIGHT,SQUARE_WHITEBISHOP,SQUARE_WHITEROOK,SQUARE_WHITEQUEEN,SQUARE_WHITEKING,
SQUARE_EMPTY },{ SQUARE_BLACKPAWN,SQUARE_BLACKKNIGHT,SQUARE_BLACKBISHOP,SQUARE_BLACKROOK,SQUARE_BLACKQUEEN,SQUARE_BLACKKING,SQUARE_EMPTY } };

int Plus8[2][64];
int Minus8[2][64];

Bitset Pos2Bit[64];

Bitset KnightMoves[64];
Bitset PawnMoves[2][64];
Bitset PawnAttacks[2][64];
Bitset KingMoves[64];

int Rank[64] = { 0,0,0,0,0,0,0,0,
1,1,1,1,1,1,1,1,
2,2,2,2,2,2,2,2,
3,3,3,3,3,3,3,3,
4,4,4,4,4,4,4,4,
5,5,5,5,5,5,5,5,
6,6,6,6,6,6,6,6,
7,7,7,7,7,7,7,7 };
int File[64] = { 7,6,5,4,3,2,1,0,
7,6,5,4,3,2,1,0,
7,6,5,4,3,2,1,0,
7,6,5,4,3,2,1,0,
7,6,5,4,3,2,1,0,
7,6,5,4,3,2,1,0,
7,6,5,4,3,2,1,0,
7,6,5,4,3,2,1,0 };

int getSquare(int file, int rank)
{
	return (8 * rank + (7 - file));
}

std::string Int2Sq(int n)
{
	std::string s = "";
	char c = getFile(n) + 97;
	char d = getRank(n) + 49;
	s += c;
	s += d;
	return s;
}

int Sq2Int(std::string s)
{
	int n = 0;
	n += (s.at(1) - 49) * 8;
	n += 7 - (s.at(0) - 97);
	if (n < 0 || n > 63)
	{
		return -1;
	}
	return n;
}

void datainit()
{
	//Plus 8 and Minus 8
	for (int j = 0; j < 64; j++)
	{
		Plus8[COLOR_WHITE][j] = std::min(j + 8, 63);
		Minus8[COLOR_BLACK][j] = std::min(j + 8, 63);
		Plus8[COLOR_BLACK][j] = std::max(j - 8, 0);
		Minus8[COLOR_WHITE][j] = std::max(j - 8, 0);
	}

	//Simple Piece Movement
	for (int i = 0; i<64; i++)
	{
		if (i == 0)
			Pos2Bit[0] = 0x1;
		else
			Pos2Bit[i] = Pos2Bit[i - 1] << 1;
		Bitset b = Pos2Bit[i];
		int f = getFile(i);
		int r = getRank(i);

		//King Moves
		if (f>0 && f<7)
		{
			if (r>0 && r<7)
				KingMoves[i] = (b << 9) | (b << 8) | (b << 7) | (b << 1) | (b >> 1) | (b >> 7) | (b >> 8) | (b >> 9);
			else if (r == 0)
				KingMoves[i] = (b << 9) | (b << 8) | (b << 7) | (b << 1) | (b >> 1);
			else if (r == 7)
				KingMoves[i] = (b << 1) | (b >> 1) | (b >> 7) | (b >> 8) | (b >> 9);
		}
		else if (f == 0)
		{
			if (r>0 && r<7)
				KingMoves[i] = (b << 8) | (b << 7) | (b >> 1) | (b >> 8) | (b >> 9);
			else if (r == 0)
				KingMoves[i] = (b << 8) | (b << 7) | (b >> 1);
			else if (r == 7)
				KingMoves[i] = (b >> 1) | (b >> 8) | (b >> 9);
		}
		else if (f == 7)
		{
			if (r>0 && r<7)
				KingMoves[i] = (b << 9) | (b << 8) | (b << 1) | (b >> 7) | (b >> 8);
			else if (r == 0)
				KingMoves[i] = (b << 9) | (b << 8) | (b << 1);
			else if (r == 7)
				KingMoves[i] = (b << 1) | (b >> 7) | (b >> 8);
		}

		//Pawn Moves
		PawnMoves[COLOR_WHITE][i] = (b << 8);
		PawnMoves[COLOR_BLACK][i] = (b >> 8);

		//Pawn Attacks
		if (f == 0)
		{
			PawnAttacks[COLOR_WHITE][i] = (b << 7);
			PawnAttacks[COLOR_BLACK][i] = (b >> 9);
		}
		else if (f == 7)
		{
			PawnAttacks[COLOR_WHITE][i] = (b << 9);
			PawnAttacks[COLOR_BLACK][i] = (b >> 7);
		}
		else
		{
			PawnAttacks[COLOR_WHITE][i] = (b << 9) | (b << 7);
			PawnAttacks[COLOR_BLACK][i] = (b >> 9) | (b >> 7);
		}

		//Knight Moves
		if (f == 0)
		{
			if (r == 0)
				KnightMoves[i] = (b << 15) | (b << 6);
			else if (r == 1)
				KnightMoves[i] = (b << 15) | (b << 6) | (b >> 10);
			else if (r == 7)
				KnightMoves[i] = (b >> 17) | (b >> 10);
			else if (r == 6)
				KnightMoves[i] = (b >> 17) | (b >> 10) | (b << 6);
			else
				KnightMoves[i] = (b << 15) | (b << 6) | (b >> 10) | (b >> 17);
		}
		else if (f == 1)
		{
			if (r == 0)
				KnightMoves[i] = (b << 17) | (b << 15) | (b << 6);
			else if (r == 1)
				KnightMoves[i] = (b << 17) | (b << 15) | (b << 6) | (b >> 10);
			else if (r == 7)
				KnightMoves[i] = (b >> 17) | (b >> 15) | (b >> 10);
			else if (r == 6)
				KnightMoves[i] = (b >> 17) | (b >> 15) | (b >> 10) | (b << 6);
			else
				KnightMoves[i] = (b << 17) | (b << 15) | (b << 6) | (b >> 10) | (b >> 15) | (b >> 17);
		}
		else if (f == 7)
		{
			if (r == 0)
				KnightMoves[i] = (b << 17) | (b << 10);
			else if (r == 1)
				KnightMoves[i] = (b << 17) | (b << 10) | (b >> 6);
			else if (r == 7)
				KnightMoves[i] = (b >> 15) | (b >> 6);
			else if (r == 6)
				KnightMoves[i] = (b >> 15) | (b >> 6) | (b << 10);
			else
				KnightMoves[i] = (b << 17) | (b << 10) | (b >> 6) | (b >> 15);
		}
		else if (f == 6)
		{
			if (r == 0)
				KnightMoves[i] = (b << 15) | (b << 17) | (b << 10);
			else if (r == 1)
				KnightMoves[i] = (b << 15) | (b << 17) | (b << 10) | (b >> 6);
			else if (r == 7)
				KnightMoves[i] = (b >> 15) | (b >> 17) | (b >> 6);
			else if (r == 6)
				KnightMoves[i] = (b >> 15) | (b >> 17) | (b >> 6) | (b << 10);
			else
				KnightMoves[i] = (b << 15) | (b << 17) | (b << 10) | (b >> 6) | (b >> 17) | (b >> 15);
		}
		else
		{
			if (r == 0)
				KnightMoves[i] = (b << 17) | (b << 15) | (b << 10) | (b << 6);
			else if (r == 1)
				KnightMoves[i] = (b << 17) | (b << 15) | (b << 10) | (b << 6) | (b >> 6) | (b >> 10);
			else if (r == 7)
				KnightMoves[i] = (b >> 6) | (b >> 10) | (b >> 15) | (b >> 17);
			else if (r == 6)
				KnightMoves[i] = (b << 10) | (b << 6) | (b >> 6) | (b >> 10) | (b >> 15) | (b >> 17);
			else
				KnightMoves[i] = (b << 17) | (b << 15) | (b << 10) | (b << 6) | (b >> 6) | (b >> 10) | (b >> 15) | (b >> 17);
		}
	}
}

void printBitset(Bitset b)
{
	for (int i = 0; i<64; i++)
	{
		if (i % 8 == 0 && i != 0)
			std::cout << std::endl;
		std::cout << (b >> 63);
		b = (b << 1);
	}
	std::cout << std::endl;
}

void printBitsetDiag(Bitset b)
{
	int sp = 7;
	for (int j = 0; j<sp; j++)
		std::cout << " ";
	for (int i = 0; i<64; i++)
	{
		if (i == 1 || i == 3 || i == 6 || i == 10 || i == 15 || i == 21 || i == 28 || i == 36 || i == 43 || i == 49 || i == 54 || i == 58 || i == 61 || i == 63)
		{
			if (i<32)
				sp--;
			else
				sp++;
			std::cout << std::endl;
			for (int j = 0; j<sp; j++)
				std::cout << " ";
		}
		std::cout << (b >> 63) << " ";
		b = (b << 1);
	}
	std::cout << std::endl;
}