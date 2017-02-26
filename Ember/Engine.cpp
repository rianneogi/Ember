#include "Engine.h"

Engine::Engine()
{
	CurrentPos.setStartPos();
}

Engine::~Engine()
{
}

void Engine::go()
{
}

int Engine::AlphaBeta()
{
	return 0;
}

uint64_t Engine::perft(int depth)
{
	if (depth == 0) return 1;

	uint64_t count = 0;
	std::vector<Move> vec;
	vec.reserve(128);
	CurrentPos.generateMoves(vec);

	auto Key = CurrentPos.HashKey;

	for (unsigned int i = 0; i < vec.size(); i++)
	{
		Move m = vec[i];
		CurrentPos.makeMove(m);
		count += perft(depth-1);
		CurrentPos.unmakeMove(m);
	}

	assert(Key == CurrentPos.HashKey);

	return count;
}
