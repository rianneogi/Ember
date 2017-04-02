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

	void addMove(std::vector<Move>& vec, Move const& m)
	{
		//makeMove(m);
		//if (!underCheck(getOpponent(Turn)))
		//{
		vec.push_back(m);
		//printf("added move %s\n", m.toString());
		//}
		//unmakeMove(m);
	}

	void setStartPos();
	void initializeBitsets();
	void clearBoard();
	void placePiece(int square, int location);
	void loadFromFEN(std::string fen);

	void generateMoves(std::vector<Move>& moves);
	
	void makeMove(const Move& m);
	bool tryMove(const Move& m)
	{
		makeMove(m);
		if (underCheck(getOpponent(Turn)))
		{
			unmakeMove(m);
			return false;
		}
		return true;
	}
	void unmakeMove(const Move& m);

	void takebackMove()
	{
		unmakeMove(movelist[movelist.size() - 1]);
	}

	bool isMoveLegal(const Move& m)
	{
		if (tryMove(m))
		{
			unmakeMove(m);
			return true;
		}
		return false;
	}
	bool isAttacked(int turn, int n) const;
	bool underCheck(int turn) const
	{
		unsigned long k = 0;
		BitscanForward(&k, Pieces[turn][PIECE_KING]);
		return isAttacked(turn, k);
	}

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

std::string getStringToken(std::string str, char delimiter, int token);
int getStringTokenPosition(std::string str, char delimiter, int token);

inline bool isStatusDraw(int status)
{
	return (status == STATUS_3FOLDREP || status == STATUS_INSUFFICIENTMAT || status == STATUS_STALEMATE);
}
