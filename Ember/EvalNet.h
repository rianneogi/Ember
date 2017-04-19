#pragma once

#include "PositionNN.h"

#include <Neurons\FullyConnectedNeuron.h>
#include <Neurons\ConvNeuron.h>
#include <Neurons\KingNeuron.h>
#include <Neurons\FileNeuron.h>
#include <Neurons\DiagNeuron.h>
#include <Neurons\Im2ColNeuron.h>
#include <Neurons\TanhNeuron.h>
#include <Neurons\SigmoidNeuron.h>
#include <Neurons\LeakyReLUNeuron.h>
#include <Neurons\ReshapeNeuron.h>
#include <ErrorFunctions\MeanSquaredError.h>
#include <ErrorFunctions\L1Error.h>
#include <ErrorFunctions\UnitError.h>
#include <Optimizers\StandardOptimizer.h>
#include <Optimizers\AdamOptimizer.h>

extern const size_t POSITION_TENSOR_SIZE;
extern const size_t MOVE_TENSOR_SIZE;

class EvalNet
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

	EvalNet();
	EvalNet(uint64_t batch_size);
	~EvalNet();

	void init_net();

	Float train(Tensor* input_pos, Tensor* input_move, Tensor* output_eval, Tensor* output_move);

	Float get_eval(Tensor input);

	void save(std::string filename);
	void load(std::string filename);
};

void moveToTensor(Move m, Tensor* t);
Move tensorToMove(Tensor* tensor);