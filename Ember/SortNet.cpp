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
	Input = mBoard->newBlob(make_shape(BatchSize, 64 + 64 + 6 + 7 + 7));
	Blob* hidden = mBoard->newBlob(make_shape(BatchSize, 32));
	Blob* hidden_act = mBoard->newBlob(make_shape(BatchSize, 32));
	Output = mBoard->newBlob(make_shape(BatchSize, 1));

	mBoard->setOptimizer(new StandardOptimizer(1.0));

	mBoard->addNeuron(new FullyConnectedNeuron(Input, Output, 1));
	//mBoard->addNeuron(new LeakyReLUNeuron(hidden, hidden_act, 0.05));
	//mBoard->addNeuron(new FullyConnectedNeuron(hidden_act, Output, 1));

	mBoard->addErrorFunction(new L1Error(Input, Output));
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