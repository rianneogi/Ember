#pragma once

#include "PositionNN.h"

#include <Neurons\FullyConnectedNeuron.h>
#include <Neurons\ConvNeuron.h>
#include <Neurons\KingNeuron.h>
#include <Neurons\FileNeuron.h>
#include <Neurons\Im2ColNeuron.h>
#include <Neurons\TanhNeuron.h>
#include <Neurons\LeakyReLUNeuron.h>
#include <ErrorFunctions\MeanSquaredError.h>
#include <ErrorFunctions\L1Error.h>
#include <ErrorFunctions\UnitError.h>
#include <Optimizers\StandardOptimizer.h>
#include <Optimizers\AdamOptimizer.h>

class Net
{
public:
	Board* mBoard;

	Blob* Input;

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

	Float get_eval(Tensor input);

	void save(std::string filename);
	void load(std::string filename);
};

void moveToTensor(Move m, Tensor* t);
void moveToTensorPtr(Move m, Float* t);
Move tensorToMove(Tensor* tensor);