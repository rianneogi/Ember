#pragma once
#include "Move.h"

extern int CONS_TTUNKNOWN;

extern Bitset TT_PieceKey[2][6][64];
extern Bitset TT_ColorKey;
extern Bitset TT_CastlingKey[2][2];
extern Bitset TT_EPKey[64];

void TTinit();
Bitset TTRandom();

