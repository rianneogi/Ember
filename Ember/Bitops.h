#pragma once

#include <string>
#include <assert.h>
#include <algorithm>
#include <iostream>

typedef uint64_t Bitset;

#define USE_SSE

enum PIECES { PIECE_PAWN, PIECE_KNIGHT, PIECE_BISHOP, PIECE_ROOK, PIECE_QUEEN, PIECE_KING, PIECE_NONE };
enum SQUARES {
	SQUARE_EMPTY, SQUARE_WHITEPAWN, SQUARE_WHITEKNIGHT, SQUARE_WHITEBISHOP, SQUARE_WHITEROOK,
	SQUARE_WHITEQUEEN, SQUARE_WHITEKING, SQUARE_BLACKPAWN, SQUARE_BLACKKNIGHT,
	SQUARE_BLACKBISHOP, SQUARE_BLACKROOK, SQUARE_BLACKQUEEN, SQUARE_BLACKKING
};
enum COLORS { COLOR_WHITE, COLOR_BLACK, COLOR_NONE };
enum CASTLE { CASTLE_KINGSIDE, CASTLE_QUEENSIDE };

extern Bitset Pos2Bit[64];

extern Bitset KnightMoves[64];
extern Bitset PawnMoves[2][64];
extern Bitset PawnAttacks[2][64];
extern Bitset KingMoves[64];

extern int Square2Piece[13];
extern int Square2Color[13];
extern int Piece2Square[2][7];

extern int Plus8[2][64];
extern int Minus8[2][64];

extern int Rank[64];
extern int File[64];

std::string Int2Sq(int n);
int Sq2Int(std::string s);

int getSquare(int file, int rank);

void datainit();

void printBitset(Bitset b);
void printBitsetDiag(Bitset b);

inline Bitset getPos2Bit(int n)
{
	//return Pos2Bit[n];
	return (1ULL << n);
}

inline int getOpponent(int n)
{
	//return Opponent[n];
	return !n;
}

inline int getSquare2Piece(int n)
{
	return Square2Piece[n];
}

inline int getSquare2Color(int n)
{
	return Square2Color[n];
}

inline int getPiece2Square(int n, int turn)
{
	return Piece2Square[turn][n];
}

inline int getPlus8(int col, int n)
{
	return Plus8[col][n];
	//return n + 8;
}

inline int getMinus8(int col, int n)
{
	//return n - 8;
	return Minus8[col][n];
}

inline Bitset getKnightMoves(int n)
{
	return KnightMoves[n];
}

inline Bitset getPawnMoves(int color, int n)
{
	return PawnMoves[color][n];
}

inline Bitset getPawnAttacks(int color, int n)
{
	return PawnAttacks[color][n];
}

inline Bitset getKingMoves(int n)
{
	return KingMoves[n];
}

inline int getFile(unsigned long n)
{
	return File[n];
	//return (n & 0x7);
}

//inline int getFile(int n)
//{
//	return File[n];
//	//return (n & 0x7);
//}

inline int getRank(unsigned long n)
{
	//return Rank[n];
	return (n >> 3);
}

//inline int getRank(int n)
//{
//	//return Rank[n];
//	return ((unsigned)(n) >> 3);
//}

inline unsigned long long popcnt(Bitset b)
{
#ifdef USE_SSE
	return _mm_popcnt_u64(b);
#else
	return popcnt2(b);
#endif
}

inline void BitscanForward(unsigned long* n, Bitset b)
{
#ifdef USE_SSE
	_BitScanForward64(n, b);
#else
	*n = lastOf(b);
#endif
}
