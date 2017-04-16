#pragma once

#include <Board.h>
#include "PGNData.h"

class PositionNN
{
public:
	Tensor Squares;
	Tensor OtherData; //EpSquare and Castling and Turn

	PositionNN();
	PositionNN(const Position& pos);
	~PositionNN();

	void copyFromPosition(const Position& pos);
	void copyToPosition(Position& pos) const;

	std::string toFEN();
};

void pos2posNN(Float* posNN, const Position& pos);
