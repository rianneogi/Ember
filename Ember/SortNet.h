#pragma once

#include "Net.h"

class SortNet
{
public:
	Board* mBoard;

	Blob* Input;
	Blob* Output;

	SortNet();
	~SortNet();

	void init_net();

	Float train(Tensor input, Tensor output);
};

