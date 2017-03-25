#pragma once

#include <Board.h>
#include "Position.h"

class PositionNN
{
public:
	Tensor mTensor;

	PositionNN();
	PositionNN(const Position& pos);
	~PositionNN();

	void copyFromPosition(const Position& pos);
};

