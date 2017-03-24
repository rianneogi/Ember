#pragma once

#include "PositionNN.h"

class Net
{
public:
	Board* mBoard;

	Blob* Input;
	Blob* ConvKing;
	Blob* FCKing;
	Blob* ActKing;
	Blob* FullFC1;
	Blob* FullFCAct1;
	Blob* FullFC2;
	Blob* Output;

	uint64_t BatchSize;

	Net();
	~Net();

	void init_net();

	void train(Tensor inputs, Tensor outputs);
};

void moveToTensor(Move m, Tensor* t);