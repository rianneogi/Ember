#include "PositionNN.h"

PositionNN::PositionNN() : Squares(make_shape(8, 8, 14)), OtherData(make_shape(69))
{
}

PositionNN::PositionNN(const Position& pos) : Squares(make_shape(8, 8, 14)), OtherData(make_shape(69))
{
	copyFromPosition(pos);
}

PositionNN::~PositionNN()
{
	Squares.freemem();
}

void PositionNN::copyFromPosition(const Position& pos)
{
	Squares.setzero();
	OtherData.setzero();
	for (int i = 0; i < 64; i++)
	{
		Squares(i / 8, i % 8, pos.Squares[i]) = 1.0;
	}

	OtherData(0) = pos.Turn;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			OtherData(1 + i * 2 + j) = pos.Castling[i][j];
		}
	}
	OtherData(5 + pos.EPSquare) = 1.0;
}

void PositionNN::copyToPosition(Position& pos) const
{
	pos.clearBoard();
	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 14; j++)
		{
			if (Squares(i / 8, i % 8, j) == 1.0)
			{
				pos.Squares[i] = j;
				break;
			}
		}
	}

	pos.Turn = OtherData(0);
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			pos.Castling[i][j] = OtherData(1 + 2 * i + j);
		}
	}
	for (int i = 5; i < 69; i++)
	{
		if (OtherData(i) == 1.0)
		{
			pos.EPSquare = i-5;
			break;
		}
	}
}

std::string PositionNN::toFEN()
{
	std::string res = "";
	int empty_count = 0;
	for (int i = 63; i >= 0; i--)
	{
		if (Squares(i/8, i%8, SQUARE_EMPTY) == 1.0)
			empty_count++;
		else
		{
			if (empty_count > 0)
				res += std::to_string(empty_count);
			for (int j = 0; j < 14; j++)
			{
				if (Squares(i / 8, i % 8, j) == 1.0)
					res += PieceStrings[j].at(0);
			}
			empty_count = 0;
		}
	}
	return res;
}

void pos2posNN(Float* posNN, const Position& pos)
{
	memset(posNN, 0, 14 * 8 * 8);
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			posNN[i * 14 * 8 + j * 14 + pos.Squares[i*8 + j]] = 1.0;
		}
	}
}
