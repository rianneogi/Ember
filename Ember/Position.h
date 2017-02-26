#pragma once

#include "TranspositionTable.h"

#include <vector>

enum GAMESTATUS { STATUS_NOTOVER, STATUS_WHITEMATED, STATUS_BLACKMATED, STATUS_STALEMATE, STATUS_INSUFFICIENTMAT, STATUS_3FOLDREP };

class Position
{
public:
	int Turn;

	Bitset Squares[64];
	Bitset Pieces[2][6];

	Bitset OccupiedSq;

	Bitset Castling[2][2];
	Bitset EPSquare;

	Bitset HashKey;

	std::vector<Move> movelist;
	std::vector<Bitset> hashlist;

	Position();
	~Position();

	void addMove(std::vector<Move>& vec, Move const& m) const;

	void setStartPos();
	void initializeBitsets();
	void generateMoves(std::vector<Move>& moves) const;
	
	void makeMove(const Move& m);
	void unmakeMove(const Move& m);

	bool isLegalMove(const Move& m) const;
	bool isAttacked(int turn, int n) const;
	bool underCheck(int turn) const;

	int getGameStatus();
	bool isRepetition();

	void display(int flip);
};

inline Bitset getRookAttacks(int sq, Bitset occ)
{
	Magic* m = &RookMagicTable[sq];
	occ = ((occ&m->mask)*m->magic) >> m->shift;
	return RookAttacks[sq][occ];
}

inline Bitset getBishopAttacks(int sq, Bitset occ)
{
	Magic* m = &BishopMagicTable[sq];
	occ = ((occ&m->mask)*m->magic) >> m->shift;
	return BishopAttacks[sq][occ];
}

inline Bitset getQueenAttacks(int sq, Bitset occ)
{
	Bitset m = getRookAttacks(sq, occ);
	m |= getBishopAttacks(sq, occ);
	return m;
}
