#include "PositionNN.h"

PositionNN::PositionNN() : mTensor(make_shape(8, 8, 14))
{
}

PositionNN::PositionNN(const Position& pos) : mTensor(make_shape(8, 8, 14))
{
	copyFromPosition(pos);
}

PositionNN::~PositionNN()
{
	mTensor.freemem();
}

void PositionNN::copyFromPosition(const Position& pos)
{
	mTensor.setzero();
	for (int i = 0; i < 64; i++)
	{
		mTensor(i / 8, i % 8, pos.Squares[i]) = 1.0;
	}
}
