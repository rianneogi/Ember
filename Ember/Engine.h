#pragma once

#include "Position.h"

class Engine
{
public:
	Position CurrentPos;

	Engine();
	~Engine();

	void go();
	int AlphaBeta();

	int LeafEval();
	int QuiescenceSearch();

	uint64_t perft(int depth);
};

