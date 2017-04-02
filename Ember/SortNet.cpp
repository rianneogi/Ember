#include "SortNet.h"

SortNet::SortNet() : BatchSize(1)
{
	init_net();
}

SortNet::SortNet(uint64_t batch_size) : BatchSize(batch_size)
{
	init_net();
}

SortNet::~SortNet()
{
	delete mBoard;
	mBoard = NULL;
}

void SortNet::init_net()
{
	mBoard = new Board();
	Input = mBoard->newBlob(make_shape(BatchSize, 1, 64 + 64 + 6 + 7 + 7));
	Output = mBoard->newBlob(make_shape(BatchSize, 1, 1));

	mBoard->setOptimizer(new AdamOptimizer(0.001));

	mBoard->addNeuron(new FullyConnectedNeuron(Input, Output, 1));

	mBoard->addErrorFunction(new MeanSquaredError(Input, Output));
}

Float SortNet::train(Tensor input, Tensor output)
{
	Float error = mBoard->backprop(input, output);
	mBoard->mOptimizer->optimize();
	return error;
}

void moveToTensorPtr(Move m, Float* t)
{
	memset(t, 0, sizeof(Float) * (64 + 64 + 6 + 7 + 7));
	t[m.getFrom()] = 1;
	t[64 + m.getTo()] = 1;

	t[128 + m.getMovingPiece()] = 1;
	t[128 + 6 + getSquare2Piece(m.getCapturedPiece())] = 1;
	t[128 + 12 + m.getSpecial()] = 1;
}