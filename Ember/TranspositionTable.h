#pragma once
#include "Move.h"

extern int CONS_TTUNKNOWN;

enum BOUNDTYPE { TT_EXACT, TT_ALPHA, TT_BETA };

extern Bitset TT_PieceKey[2][6][64];
extern Bitset TT_ColorKey;
extern Bitset TT_CastlingKey[2][2];
extern Bitset TT_EPKey[64];

struct HashEntry
{
	Bitset key;
	Move bestmove;
	int depth;
	int score;
	int bound;
	bool avoidnull;

	HashEntry() : key(0), depth(0), score(CONS_TTUNKNOWN), bound(TT_EXACT) {}
	HashEntry(Bitset k, int d, int s, int b, Move m) : key(k), depth(d), score(s), bound(b), bestmove(m) {}
};

struct ProbeStruct
{
	HashEntry* entry;
	int score;
	bool found;
};

class TranspositionTable
{
public:
	HashEntry* Entries;
	uint64_t Size; //Size needs to be a power of 2
	uint64_t SizeMinusOne;

	uint64_t hits;

	TranspositionTable(uint64_t s);
	~TranspositionTable();

	void Save(HashEntry const& entry);
	void Save(Bitset key, int depth, int score, int bound, Move bestmove);
	int Probe(Bitset key, int depth, int alpha, int beta);
	Move getBestMove(Bitset key);
};

void TTinit();
Bitset TTRandom();

