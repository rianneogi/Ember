#include "TranspositionTable.h"

Bitset TT_PieceKey[2][6][64];
Bitset TT_ColorKey;
Bitset TT_CastlingKey[2][2];
Bitset TT_EPKey[64];

int CONS_TTUNKNOWN = -32767;

void TTinit()
{
	for (int i = 0; i< 2; i++)
	{
		for (int j = 0; j<6; j++)
		{
			for (int k = 0; k<64; k++)
			{
				TT_PieceKey[i][j][k] = TTRandom();
			}
		}
	}

	TT_ColorKey = TTRandom();

	for (int i = 0; i<2; i++)
	{
		for (int j = 0; j<2; j++)
		{
			TT_CastlingKey[i][j] = TTRandom();
		}
	}

	for (int i = 0; i<64; i++)
	{
		TT_EPKey[i] = TTRandom();
	}
}

Bitset TTRandom()
{
	unsigned long long r0 = rand(), r1 = rand(), r2 = rand(), r3 = rand();
	return (r0 | (r1 << 16ULL) | (r2 << 32ULL) | (r3 << 48ULL));
	//return (rand() | (rand()<<16) | (rand()<<32) | (rand()<<48));
}

TranspositionTable::TranspositionTable(uint64_t s)
{
	Size = s;
	Entries = new HashEntry[Size];
	SizeMinusOne = Size - 1;
	hits = 0;
}

TranspositionTable::~TranspositionTable()
{
	delete[] Entries;
	Entries = NULL;
}

void TranspositionTable::Save(HashEntry const& entry)
{
	HashEntry* hash = &Entries[entry.key&SizeMinusOne];

	if (hash->key != entry.key
		|| entry.depth >= hash->depth
		)
	{
		hash->key = entry.key;
		hash->score = entry.score;
		hash->depth = entry.depth;
		hash->bound = entry.bound;
		hash->bestmove = entry.bestmove;
	}
}

void TranspositionTable::Save(Bitset key, int depth, int score, int bound, Move bestmove)
{
	HashEntry* hash = &Entries[key&SizeMinusOne];

	if (hash->key != key
		|| depth >= hash->depth
		//|| ((bound != TT_EXACT && hash->bound != TT_EXACT) || (bound==TT_EXACT && hash->bound==TT_EXACT)
		//	|| (bound==TT_EXACT && hash->bound!=TT_EXACT))
		//|| !(!hash->bestmove.isNullMove() && bestmove.isNullMove())
		)
	{
		hash->key = key;
		hash->score = score;
		hash->depth = depth;
		hash->bound = bound;
		hash->bestmove = bestmove;
	}
}

int TranspositionTable::Probe(Bitset key, int depth, int alpha, int beta)
{
	HashEntry* hash = &Entries[key&SizeMinusOne];
	if (hash->key == key)
	{
		if ((hash->depth >= depth))
		{
			if (hash->bound == TT_EXACT)
				return hash->score;
			if (hash->bound == TT_ALPHA && hash->score <= alpha)
				return hash->score;
			if (hash->bound == TT_BETA && hash->score >= beta)
				return hash->score;
		}
	}
	return CONS_TTUNKNOWN;
}

Move TranspositionTable::getBestMove(Bitset key)
{
	HashEntry* hash = &Entries[key&SizeMinusOne];
	if (key == hash->key)
		return hash->bestmove;
	return CONST_NULLMOVE;
}
