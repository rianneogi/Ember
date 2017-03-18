#include "PositionNN.h"

PositionNN::PositionNN() : mData(make_shape(14, 8, 8))
{
	mData.setzero();
}

PositionNN::PositionNN(const Position& pos) : PositionNN()
{
	copyFromPosition(pos);
}

PositionNN::~PositionNN()
{
	mData.freemem();
}

void PositionNN::copyFromPosition(const Position& pos)
{
	for (int i = 0; i < 64; i++)
	{
		mData(pos.Squares[i], i / 8, i % 8) = 1;
	}
}
