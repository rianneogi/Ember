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
	Blob* OutputMoveFC;
	Blob* OutputEvalFC;
	Blob* Output_Move;
	Blob* Output_Eval;

	uint64_t BatchSize;

	Net();
	Net(uint64_t batch_size);
	~Net();

	void init_net();

	Float train(Tensor inputs, Tensor* output_move, Tensor* output_eval);

	void save(std::string filename);
	void load(std::string filename);
};

void moveToTensor(Move m, Tensor* t);