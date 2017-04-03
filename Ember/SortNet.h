#pragma once

#include "EvalNet.h"

class SortNet
{
public:
	Board* mBoard;

	Blob* Input;
	Blob* Output;

	uint64_t BatchSize;

	SortNet();
	SortNet(uint64_t batch_size);
	~SortNet();

	void init_net();

	Float train(Tensor input, Tensor output);
};

void moveToTensorPtr(Move m, Float* t);

