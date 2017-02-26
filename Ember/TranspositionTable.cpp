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
