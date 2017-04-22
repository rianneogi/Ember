#pragma once

#include "EvalNet.h"

class SortNet
{
public:
	Board* mBoard;

	Blob* Input_Pos;
	Blob* Input_Move;

	Blob* ConvKing;
	Blob* FCKing;
	Blob* ActKing;

	Blob* FCRank;
	Blob* ActRank;

	Blob* ConvFile;
	Blob* FCFile;
	Blob* ActFile;

	Blob* ConvDiag;
	Blob* FCDiag;
	Blob* ActDiag;

	Blob* ConvKnight;
	Blob* FCKnight;
	Blob* ActKnight;

	Blob* MoveFC;
	Blob* MoveFCAct;

	Blob* ConvFull;
	Blob* ConvAct;

	Blob* FullFC;
	Blob* FullFCAct;

	Blob* OutputEvalFC;
	Blob* Output_Eval;

	Blob* OutputMoveFC;
	Blob* Output_Move;

	ConvNeuron* Conv;
	FullyConnectedNeuron* FC1;

	uint64_t BatchSize;

	SortNet();
	SortNet(uint64_t batch_size);
	~SortNet();

	void init_net();

	Float train(Tensor* input_pos, Tensor* input_move, Tensor* output_eval, Tensor* output_move);

	void save(std::string filename);
	void load(std::string filename);
};

void moveToTensorPtr(const Move& m, Float* t);

